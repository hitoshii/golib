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
#ifndef __JLIB_XPOLL_H__
#define __JLIB_XPOLL_H__

#include "jtypes.h"
#include "jarray.h"
#include <sys/epoll.h>

typedef struct _JXPoll JXPoll;

typedef enum {
    J_XPOLL_IN = EPOLLIN,
    J_XPOLL_OUT = EPOLLOUT,
    J_XPOLL_RDHUP = EPOLLRDHUP, /* Steam socket peer closed connection, or shutdown writing */
    J_XPOLL_PRI = EPOLLPRI,     /* There is urgent data available for read */
    J_XPOLL_ERR = EPOLLERR,
    J_XPOLL_HUP = EPOLLHUP,
    J_XPOLL_ET = EPOLLET,       /* Edge Triggered */
    J_XPOLL_ONESHOT = EPOLLONESHOT,
} JXPollCondition;

typedef enum {
    J_XPOLL_CTL_ADD = EPOLL_CTL_ADD,
    J_XPOLL_CTL_MOD = EPOLL_CTL_MOD,
    J_XPOLL_CTL_DEL = EPOLL_CTL_DEL,
} JXPollControl;

typedef struct {
    int fd;
    unsigned short events;
    void * user_data;
} JXPollEvent;

JXPoll *j_xpoll_new(void);
void j_xpoll_close(JXPoll *p);

boolean j_xpoll_add(JXPoll *p, int fd, unsigned short events, void * user_data);
boolean j_xpoll_del(JXPoll *p, int fd, unsigned short events);

int j_xpoll_wait(JXPoll *p, JXPollEvent *events, unsigned int maxevent, int timeout);

unsigned int j_xpoll_event_count(JXPoll *p);

#endif
