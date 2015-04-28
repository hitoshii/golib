/*
 * Copyright (C) 2015  Wiky L
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "jepoll.h"
#include "jmem.h"
#include "jhashtable.h"
#include <unistd.h>

struct _JEPoll {
    jint pfd;
    JHashTable *fds;
};
#define j_epoll_get_fd(ep)  ((ep)->pfd)
#define j_epoll_get_fds(ep) ((ep)->fds)


JEPoll *j_epoll_new(void)
{
    jint fd = epoll_create(128);    /* size is ignored since linux 2.6 */
    if (J_UNLIKELY(fd < 0)) {
        return NULL;
    }
    JEPoll *p = (JEPoll *) j_malloc(sizeof(JEPoll));
    p->pfd = fd;
    p->fds = j_hash_table_new(16, j_int_hash, j_int_equal, NULL, NULL);
    return p;
}

void j_epoll_close(JEPoll * p)
{
    close(j_epoll_get_fd(p));
    j_free(p);
}

static inline jboolean j_epoll_add(JEPoll * p, jint fd, jushort events,
                                   jpointer data)
{
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

static inline jboolean j_epoll_mod(JEPoll * p, jint fd, jushort events,
                                   jpointer data, JDestroyNotify destroy)
{
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

static inline jboolean j_epoll_del(JEPoll * p, jint fd,
                                   JDestroyNotify destroy)
{
    jpointer old_data =
        j_hash_table_remove(j_epoll_get_fds(p), JINT_TO_JPOINTER(fd));
    destroy(old_data);
    struct epoll_event event;
    if (epoll_ctl(j_epoll_get_fd(p), J_EPOLL_CTL_DEL, fd, &event)) {
        return FALSE;
    }
    return TRUE;
}

jboolean j_epoll_ctl(JEPoll * p, jint fd, JEPollControl ctl,
                     jushort events, jpointer data, JDestroyNotify destroy)
{
    if (ctl == J_EPOLL_CTL_ADD) {
        return j_epoll_add(p, fd, events, data);
    } else if (ctl == J_EPOLL_CTL_DEL) {
        return j_epoll_del(p, fd, destroy);
    } else {
        return j_epoll_mod(p, fd, events, data, destroy);
    }
}
