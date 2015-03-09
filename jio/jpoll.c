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
#include "jpoll.h"
#include <stdlib.h>
#include <unistd.h>
#include <jlib/jlib.h>

struct _JPoll {
    int fd;
};

int j_poll_get_fd(JPoll * p)
{
    return p->fd;
}


static inline JPoll *j_poll_alloc(int fd);


JPoll *j_poll_new(void)
{
    int fd = epoll_create(1);
    if (fd < 0) {
        return NULL;
    }
    JPoll *p = j_poll_alloc(fd);
    return p;
}

int j_poll_ctl(JPoll * poll, JPollOp jop, unsigned int events,
               JSocket * jsock, void *user_data)
{
    int pfd = j_poll_get_fd(poll);
    int fd = j_socket_get_fd(jsock);

    struct epoll_event event;
    JPollOp op = J_POLL_CTL_DEL;

    if (jop == J_POLL_CTL_ADD) {
        event.events = events;
        event.data.ptr = user_data;
        op = EPOLL_CTL_ADD;
    } else if (jop == J_POLL_CTL_MOD) {
        event.events = events;
        event.data.ptr = user_data;
        op = EPOLL_CTL_MOD;
    }
    return epoll_ctl(pfd, op, fd, &event) == 0;
}

int j_poll_wait(JPoll * poll, JPollEvent * events,
                unsigned int maxevents, int timeout)
{
    struct epoll_event _events[4096];
    if (maxevents > 4096) {
        maxevents = 4096;
    }
    int efd = j_poll_get_fd(poll);
    int n = epoll_wait(efd, _events, maxevents, timeout);
    if (n <= 0) {
        return n;
    }
    int i;
    for (i = 0; i < n; i++) {
        events[i].events = _events[i].events;
        events[i].socket = (JSocket *) _events[i].data.ptr;
    }
    return n;
}

/*
 * Closes a epoll and free all memory associated
 */
void j_poll_close(JPoll * poll)
{
    close(j_poll_get_fd(poll));
    j_free(poll);
}

static inline JPoll *j_poll_alloc(int fd)
{
    JPoll *poll = (JPoll *) j_malloc(sizeof(JPoll));
    poll->fd = fd;
    return poll;
}
