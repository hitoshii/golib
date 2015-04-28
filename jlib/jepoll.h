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
#ifndef __JLIB_EPOLL_H__
#define __JLIB_EPOLL_H__

#include "jtypes.h"
#include <sys/epoll.h>

typedef struct _JEPoll JEPoll;

typedef enum {
    J_EPOLL_IN = EPOLLIN,
    J_EPOLL_OUT = EPOLLOUT,
    J_EPOLL_RDHUP = EPOLLRDHUP, /* Steam socket peer closed connection, or shutdown writing */
    J_EPOLL_PRI = EPOLLPRI,     /* There is urgent data available for read */
    J_EPOLL_ERR = EPOLLERR,
    J_EPOLL_HUP = EPOLLHUP,
    J_EPOLL_ET = EPOLLET,       /* Edge Triggered */
    J_EPOLL_ONESHOT = EPOLLONESHOT,
} JEPollCondition;

typedef enum {
    J_EPOLL_CTL_ADD = EPOLL_CTL_ADD,
    J_EPOLL_CTL_MOD = EPOLL_CTL_MOD,
    J_EPOLL_CTL_DEL = EPOLL_CTL_DEL,
} JEPollControl;


/*
 * Creates a new epoll instance
 */
JEPoll *j_epoll_new(void);
void j_epoll_close(JEPoll * p);

jboolean j_epoll_ctl(JEPoll * pfd, jint fd, JEPollControl ctl,
                     jushort events, jpointer data,
                     JDestroyNotify destroy);

#endif
