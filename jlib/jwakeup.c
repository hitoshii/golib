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
#include "jwakeup.h"
#include "jmem.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(HAVE_EVENTFD)
#include <sys/eventfd.h>
#endif


struct _JWakeup {
    jint fds[2];
};

JWakeup *j_wakeup_new(void)
{
    JWakeup *wakeup = (JWakeup *) j_malloc(sizeof(JWakeup));
#if defined(HAVE_EVENTFD)
    jint fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (fd >= 0) {
        wakeup->fds[0] = fd;
        wakeup->fds[1] = -1;
        return wakeup;
    }
#endif
    if (pipe(wakeup->fds) < 0) {
        j_free(wakeup);
        return NULL;
    }
    if (fcntl(wakeup->fds[0], F_SETFD, O_CLOEXEC | O_NONBLOCK) < 0 ||
        fcntl(wakeup->fds[1], F_SETFD, O_CLOEXEC | O_NONBLOCK) < 0) {
        j_free(wakeup);
    }
    return wakeup;
}

jint j_wakeup_get_pollfd(JWakeup * wakeup, JEPollEvent * e)
{
    if (e) {
        e->fd = wakeup->fds[0];
        e->events = J_EPOLL_IN;
        e->data = NULL;
    }
    return wakeup->fds[0];
}

void j_wakeup_acknowledge(JWakeup * wakeup)
{
    char buf[16];
    while (read(wakeup->fds[0], buf, sizeof(buf)) == sizeof(buf));  /* 读取所有数据 */
}

void j_wakeup_signal(JWakeup * wakeup)
{
    jint res;
    if (wakeup->fds[1] == -1) {
        /* eventfd */
        juint64 one = 1;
        do {
            res = write(wakeup->fds[0], &one, sizeof(one));
        } while (J_UNLIKELY(res == -1 && errno == EINTR));
    } else {
        juint8 one = 1;
        do {
            res = write(wakeup->fds[1], &one, sizeof(one));
        } while (J_UNLIKELY(res == -1 && errno == EINTR));
    }
}

void j_wakeup_free(JWakeup * wakeup)
{
    close(wakeup->fds[0]);
    close(wakeup->fds[1]);
    j_free(wakeup);
}
