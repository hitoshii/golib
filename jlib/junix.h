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
#ifndef __JLIB_UNIX_H__
#define __JLIB_UNIX_H__

#include "jtypes.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


/* 一些系统调用的包裹 */
jint j_socket(jint family, jint type, jint protocol);
jint j_accept(jint sockfd, struct sockaddr *addr, socklen_t * addrlen);
jint j_connect(jint sockfd, const struct sockaddr *addr,
               socklen_t addrlen);
jint j_send(jint sockfd, const void *buf, size_t len, int flags);
jint j_sendto(jint sockfd, const void *buf, size_t len,
              int flags, const struct sockaddr *dest_addr,
              socklen_t addrlen);
jint j_recv(int sockfd, void *buf, size_t len, int flags);
jint j_recvfrom(int sockfd, void *buf, size_t len, int flags,
                struct sockaddr *src_addr, socklen_t * addrlen);

jint j_read(jint fd, void *buf, juint size);
jint j_write(jint fd, const void *buf, juint count);

#endif
