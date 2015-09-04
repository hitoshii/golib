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

#include "junix.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

/* 系统调用socket(int,int,int)的包裹函数 */
int j_socket(int family, int type, int protocol) {
    /* 设置*_CLOEXEC标志后，通过exec*函数执行的程序中不能使用该套接字 */
    int fd;
#ifdef SOCK_CLOEXEC
    fd = socket(family, type | SOCK_CLOEXEC, protocol);
    if (fd >= 0) {              /* 成功 */
        return fd;
    }
    if (errno == EINVAL || errno == EPROTOTYPE)
#endif
        fd = socket(family, type, protocol);
    if (fd < 0) {
        return -1;
    }
    int flags = fcntl(fd, F_GETFD, 0);
    if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
        flags |= FD_CLOEXEC;
        fcntl(fd, F_SETFD, flags);
    }
    return fd;
}

int j_accept(int sockfd, struct sockaddr * addr, socklen_t * addrlen) {
    int ret;
    while ((ret = accept(sockfd, addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

int j_connect(int sockfd, const struct sockaddr * addr,
              socklen_t addrlen) {
    int ret;
    while ((ret = connect(sockfd, addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

int j_send(int sockfd, const void *buf, size_t len, int flags) {
    int ret;
    while ((ret = send(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

int j_sendto(int sockfd, const void *buf, size_t len,
             int flags, const struct sockaddr * dest_addr,
             socklen_t addrlen) {
    int ret;
    while ((ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

int j_recv(int sockfd, void *buf, size_t len, int flags) {
    int ret;
    while ((ret = recv(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

int j_recvfrom(int sockfd, void *buf, size_t len, int flags,
               struct sockaddr * src_addr, socklen_t * addrlen) {
    int ret;
    while ((ret =
                recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

int j_read(int fd, void *buf, unsigned int size) {
    int ret;
    while ((ret = read(fd, buf, size)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

int j_write(int fd, const void *buf, unsigned int count) {
    int ret;
    while ((ret = write(fd, buf, count)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

pid_t j_wait(int *stat_loc) {
    pid_t ret;
    while((ret=wait(stat_loc))<0) {
        if(errno==EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

int j_open(const char *path, int oflag, ...) {
    int mode=0, fd;
    if(oflag & O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mode=va_arg(ap, int);
        va_end(ap);
    }
    while((fd=open(path, oflag, mode))<0) {
        if(errno==EINTR) {
            continue;
        }
        break;
    }
    return fd;
}
