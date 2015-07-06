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

jint j_poll(struct pollfd *fds, juint nfds, jint timeout)
{
    return poll(fds, nfds, timeout);
}

/* poll单个文件描述符 */
jint j_poll_simple(jint fd, jshort conditions, jint timeout,
                   jshort * revents)
{
    struct pollfd fds;
    fds.fd = fd;
    fds.events = conditions;
    jint ret = j_poll(&fds, 1, timeout);
    if (ret >= 0) {
        *revents = fds.revents;
    }
    return ret;
}
