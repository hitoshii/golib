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
#include "jepoll.h"
#include "jmem.h"
#include "jhashtable.h"
#include <unistd.h>
#include <errno.h>

struct _JEPoll {
    jint pfd;
    JHashTable *fds;            /* fd => user_data */

    /* 调用epoll_wait()用 */
    struct epoll_event *cached_events;
    juint n_cached_events;
};
#define j_epoll_get_fd(ep)  ((ep)->pfd)
#define j_epoll_get_fds(ep) ((ep)->fds)


JEPoll *j_epoll_new(void) {
    jint fd = epoll_create(128);    /* size is ignored since linux 2.6 */
    if (J_UNLIKELY(fd < 0)) {
        return NULL;
    }
    JEPoll *p = (JEPoll *) j_malloc(sizeof(JEPoll));
    p->pfd = fd;
    p->fds = j_hash_table_new(16, j_int_hash, j_int_equal, NULL, NULL);
    p->n_cached_events = 0;
    p->cached_events = NULL;
    return p;
}

juint j_epoll_count(JEPoll * p) {
    return j_hash_table_length(p->fds);
}

void j_epoll_close(JEPoll * p) {
    close(j_epoll_get_fd(p));
    j_hash_table_free(p->fds);
    j_free(p->cached_events);
    j_free(p);
}

jboolean j_epoll_add(JEPoll * p, jint fd, jushort events, jpointer data) {
    struct epoll_event event = {
        events,
    };
    event.data.fd = fd;
    if (epoll_ctl(j_epoll_get_fd(p), J_EPOLL_CTL_ADD, fd, &event)) {
        return FALSE;
    }
    j_hash_table_insert(j_epoll_get_fds(p), JINT_TO_JPOINTER(fd), data);
    return TRUE;
}

jboolean j_epoll_mod(JEPoll * p, jint fd, jushort events,
                     jpointer data, JDestroyNotify destroy) {
    struct epoll_event event = {
        events,
    };
    event.data.fd = fd;
    if (epoll_ctl(j_epoll_get_fd(p), J_EPOLL_CTL_MOD, fd, &event)) {
        return FALSE;
    }
    jpointer old_data =
        j_hash_table_remove(j_epoll_get_fds(p), JINT_TO_JPOINTER(fd));
    destroy(old_data);
    j_hash_table_insert(j_epoll_get_fds(p), JINT_TO_JPOINTER(fd), data);
    return TRUE;
}

jboolean j_epoll_del(JEPoll * p, jint fd, JDestroyNotify destroy) {
    jpointer old_data =
        j_hash_table_remove(j_epoll_get_fds(p), JINT_TO_JPOINTER(fd));
    if (destroy && old_data) {
        destroy(old_data);
    }
    struct epoll_event event;
    if (epoll_ctl(j_epoll_get_fd(p), J_EPOLL_CTL_DEL, fd, &event)) {
        return FALSE;
    }
    return TRUE;
}

jboolean j_epoll_ctl(JEPoll * p, jint fd, JEPollControl ctl,
                     jushort events, jpointer data, JDestroyNotify destroy) {
    if (ctl == J_EPOLL_CTL_ADD) {
        return j_epoll_add(p, fd, events, data);
    } else if (ctl == J_EPOLL_CTL_DEL) {
        return j_epoll_del(p, fd, destroy);
    } else {
        return j_epoll_mod(p, fd, events, data, destroy);
    }
}

jint j_epoll_wait(JEPoll * p, JEPollEvent * events, juint maxevent,
                  jint timeout) {
    if (J_UNLIKELY(maxevent == 0 || maxevent > 1024 * 1024)) {
        return -1;
    }

    if (p->n_cached_events < maxevent) {
        p->n_cached_events = maxevent;
        p->cached_events = j_realloc(p->cached_events,
                                     sizeof(struct epoll_event) *
                                     maxevent);
    }

    struct epoll_event *e = p->cached_events;
    jint ret;
    errno = 0;
    while ((ret = epoll_wait(j_epoll_get_fd(p), e, maxevent, timeout)) < 0) {
        if (errno != EINTR) {
            return ret;
        }
    }
    jint i;
    for (i = 0; i < ret; i++) {
        events[i].fd = e[i].data.fd;
        events[i].data = j_hash_table_find(j_epoll_get_fds(p),
                                           JINT_TO_JPOINTER(e[i].data.fd));
        events[i].events = e[i].events;
    }
    return ret;
}

jboolean j_epoll_has(JEPoll * p, jint fd) {
    return j_hash_table_find(p->fds, JINT_TO_JPOINTER(fd)) != NULL;
}

/*
 * 获取所有注册的文件描述符号
 */
JPtrArray *j_epoll_fds(JEPoll * p) {
    return j_hash_table_get_keys(p->fds);
}
