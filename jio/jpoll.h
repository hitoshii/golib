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
#ifndef __JIO_POLL_H__
#define __JIO_POLL_H__

/*
 * EPoll wrapper
 */

#include "jsocket.h"
#include <sys/epoll.h>

typedef struct _JPoll JPoll;


JPoll *j_poll_new(void);

int j_poll_get_fd(JPoll * p);

typedef enum {
    J_POLL_CTL_ADD,
    J_POLL_CTL_DEL,
    J_POLL_CTL_MOD
} JPollOp;

#define J_POLLIN EPOLLIN
#define J_POLLOUT EPOLLOUT
#define J_POLLRDHUP EPOLLRDHUP
#define J_POLLPRI EPOLLPRI
#define J_POLLERR EPOLLERR
#define J_POLLHUP EPOLLHUP
#define J_POLLET EPOLLET
#define J_POLLONESHOT  EPOLLONESHOT

typedef struct {
    unsigned int events;
    JSocket *socket;
} JPollEvent;

/*
 * wrapper of epoll_ctl
 */
int j_poll_ctl(JPoll * poll, JPollOp op, unsigned int events,
               JSocket * jsock, void *user_data);
/*
 * wrapper of epoll_wait
 * @timeout, milliseconds
 */
int j_poll_wait(JPoll * poll, JPollEvent * events,
                unsigned int maxevents, int timeout);

/*
 * Closes a epoll and free all memory associated
 */
void j_poll_close(JPoll * poll);

#endif
