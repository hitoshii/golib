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

typedef enum {
    J_POLLIN = EPOLLIN,
    J_POLLOUT = EPOLLOUT,
    J_POLLRDHUP = EPOLLRDHUP,
    J_POLLPRI = EPOLLPRI,
    J_POLLERR = EPOLLERR,
    J_POLLHUP = EPOLLHUP,
    J_POLLET = EPOLLET,
    J_POLLONESHOT = EPOLLONESHOT,
} JPollEvent;

int j_poll_ctl(JPoll * poll, JPollOp op, unsigned int event,
               JSocket * jsock);

#endif
