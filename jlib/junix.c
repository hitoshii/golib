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

/* 系统调用socket(int,int,int)的包裹函数 */
jint j_socket(jint family, jint type, jint protocol) {
    /* 设置*_CLOEXEC标志后，通过exec*函数执行的程序中不能使用该套接字 */
    jint fd;
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
    jint flags = fcntl(fd, F_GETFD, 0);
    if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
        flags |= FD_CLOEXEC;
        fcntl(fd, F_SETFD, flags);
    }
    return fd;
}

jint j_accept(jint sockfd, struct sockaddr * addr, socklen_t * addrlen) {
    jint ret;
    while ((ret = accept(sockfd, addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

jint j_connect(jint sockfd, const struct sockaddr * addr,
               socklen_t addrlen) {
    jint ret;
    while ((ret = connect(sockfd, addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

jint j_send(jint sockfd, const void *buf, size_t len, int flags) {
    jint ret;
    while ((ret = send(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

jint j_sendto(jint sockfd, const void *buf, size_t len,
              int flags, const struct sockaddr * dest_addr,
              socklen_t addrlen) {
    jint ret;
    while ((ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

jint j_recv(int sockfd, void *buf, size_t len, int flags) {
    jint ret;
    while ((ret = recv(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;

    }
    return ret;
}

jint j_recvfrom(int sockfd, void *buf, size_t len, int flags,
                struct sockaddr * src_addr, socklen_t * addrlen) {
    jint ret;
    while ((ret =
                recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

jint j_read(jint fd, void *buf, juint size) {
    jint ret;
    while ((ret = read(fd, buf, size)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}

jint j_write(jint fd, const void *buf, juint count) {
    jint ret;
    while ((ret = write(fd, buf, count)) < 0) {
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    return ret;
}
