/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */
#include "jxpoll.h"
#include "jmem.h"
#include "jhashtable.h"
#include <errno.h>
#include <unistd.h>

typedef struct {
    unsigned short event;  /* single event */
    void * user_data;
} JXPollData;

static inline JXPollData *j_xpoll_data_new(unsigned short event, void * user_data);
static inline void j_xpoll_datas_free(void * data);

struct _JXPoll {
    int fd;
    JHashTable *datas;

    unsigned int count;

    /* 调用epoll_wait()用 */
    struct epoll_event *cached_events;
    unsigned int n_cached_events;
};


JXPoll *j_xpoll_new(void) {
    int fd=epoll_create(128);
    if(fd<0) {
        return NULL;
    }
    JXPoll *p=(JXPoll*)j_malloc(sizeof(JXPoll));
    p->fd=fd;
    p->datas=j_hash_table_new(10, j_int_hash,j_int_equal, NULL, j_xpoll_datas_free);
    p->n_cached_events=0;
    p->cached_events=NULL;
    p->count=0;
    return p;
}

void j_xpoll_close(JXPoll *p) {
    close(p->fd);
    j_free(p->cached_events);
    j_hash_table_free_full(p->datas);
    j_free(p);
}


static inline JXPollData *j_xpoll_data_new(unsigned short event, void * user_data) {
    JXPollData *data=j_malloc(sizeof(JXPollData));
    data->event=event;
    data->user_data=user_data;
    return data;
}


static inline void j_xpoll_datas_free(void * data) {
    j_list_free_full(data, j_free);
}

static inline boolean j_xpoll_add_internal(JXPoll *p, JList *datas, int fd, unsigned short event) {
    unsigned short events=event;
    JList *ptr=datas;
    while(ptr) {
        JXPollData *data=(JXPollData*)j_list_data(ptr);
        if(data->event==event) {
            return FALSE;
        }
        events |= data->event;
        ptr=j_list_next(ptr);
    }
    struct epoll_event ev = {
        events,
    };
    ev.data.fd = fd;
    if(epoll_ctl(p->fd, J_XPOLL_CTL_ADD, fd, &ev)) {
        if(errno==EEXIST) {
            return epoll_ctl(p->fd, J_XPOLL_CTL_MOD, fd, &ev)==0;
        }
        return FALSE;
    }
    return TRUE;
}

boolean j_xpoll_add(JXPoll *p, int fd, unsigned short event, void * user_data) {
    JList *datas=(JList*)j_hash_table_find(p->datas, JINT_TO_JPOINTER(fd));
    if(j_xpoll_add_internal(p, datas, fd, event)) {
        JXPollData *data=j_xpoll_data_new(event, user_data);
        if(datas==NULL) {
            j_hash_table_insert(p->datas, JINT_TO_JPOINTER(fd), j_list_append(NULL, data));
        } else {
            datas=j_list_append(datas, data);
        }
        p->count++;
        return TRUE;
    }
    return FALSE;
}

static inline JList *j_xpoll_del_internal(JXPoll *p, JList *datas, int fd, unsigned short event) {
    JList *ret=NULL;
    JList *ptr=datas;
    unsigned short events=0;
    while(ptr) {
        JXPollData *data=(JXPollData*)j_list_data(ptr);
        if(data->event==event) {
            ret=ptr;
        } else {
            events|=data->event;
        }
        ptr=j_list_next(ptr);
    }
    if(ret==NULL) {
        return NULL;
    }

    struct epoll_event ev = {
        events,
    };
    ev.data.fd = fd;
    if(events) {
        epoll_ctl(p->fd, J_XPOLL_CTL_MOD, fd, &ev);
    } else {
        epoll_ctl(p->fd, J_XPOLL_CTL_DEL, fd, &ev);
    }
    return ret;
}

boolean j_xpoll_del(JXPoll *p, int fd, unsigned short event) {
    JList *datas=(JList*)j_hash_table_find(p->datas, JINT_TO_JPOINTER(fd));
    JList *link=j_xpoll_del_internal(p, datas, fd, event);
    if(link==NULL) {
        return FALSE;
    }
    j_free(j_list_data(link));
    JList *new=j_list_remove_link(datas, link);
    if(new!=datas) {
        if(new) {
            j_hash_table_update(p->datas, JINT_TO_JPOINTER(fd), new);
        } else {
            j_hash_table_remove(p->datas, JINT_TO_JPOINTER(fd));
        }
    }
    p->count--;
    return TRUE;
}


int j_xpoll_wait(JXPoll *p, JXPollEvent *events, unsigned int maxevent, int timeout) {
    if(J_UNLIKELY(maxevent==0)) {
        return 0;
    }

    if (p->n_cached_events < maxevent) {
        p->n_cached_events = maxevent;
        p->cached_events = j_realloc(p->cached_events,
                                     sizeof(struct epoll_event) *
                                     (100+maxevent));
    }

    struct epoll_event *e = p->cached_events;
    int i, j, n;
    n = epoll_wait(p->fd, e, maxevent, timeout);
    if (n<=0) {
        return n;
    }

    for (i = 0, j=0; i < n && j<maxevent; i++) {
        int fd = e[i].data.fd;
        JList *ptr = j_hash_table_find(p->datas,
                                       JINT_TO_JPOINTER(fd));
        while(ptr && j<maxevent) {
            JXPollData *data=(JXPollData*)j_list_data(ptr);
            if(data->event&e[i].events) {
                events[j].fd=fd;
                events[j].events=data->event;
                events[j++].user_data=data->user_data;
            }
            ptr=j_list_next(ptr);
        }
    }
    return j;
}

unsigned int j_xpoll_event_count(JXPoll *p) {
    return p->count;
}
