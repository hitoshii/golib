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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>


/* 一些系统调用的包裹 */
int j_socket(int family, int type, int protocol);
int j_accept(int sockfd, struct sockaddr *addr, socklen_t * addrlen);
int j_connect(int sockfd, const struct sockaddr *addr,
              socklen_t addrlen);
int j_send(int sockfd, const void *buf, size_t len, int flags);
int j_sendto(int sockfd, const void *buf, size_t len,
             int flags, const struct sockaddr *dest_addr,
             socklen_t addrlen);
int j_recv(int sockfd, void *buf, size_t len, int flags);
int j_recvfrom(int sockfd, void *buf, size_t len, int flags,
               struct sockaddr *src_addr, socklen_t * addrlen);

int j_read(int fd, void *buf, unsigned int size);
int j_write(int fd, const void *buf, unsigned int count);

pid_t j_wait(int *stat_loc);


int j_open(const char *path, int oflag, ...);

/* 创建无名管道 */
boolean j_pipe(int *pipefd);

#endif
