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
#include "jsocket.h"
#include "jpoll.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* j_socket_receive_from()的接收者缓存长度 */
#define RECV_ADDR_CACHE_SIZE 8

struct _JSocket {
    JSocketFamily family;
    JSocketType type;
    JSocketProtocol protocol;
    jint fd;

    jint listen_backlog;
    juint timeout;
    JSocketAddress remote_address;

    juint blocking:1;
    juint keepalive:1;
    juint closed:1;
    juint connected:1;
    juint listening:1;
    juint timed_out:1;
    juint connect_pending:1;

    struct {
        JSocketAddress addr;
        struct sockaddr *native;
        jint native_len;
        juint64 last_used;
    } recv_addr_cache[RECV_ADDR_CACHE_SIZE];
};

/* 系统调用socket(int,int,int)的包裹函数 */
static inline jint j_socket(jint family, jint type, jint protocol);
/* 根据文件描述符设置套接字信息 */
static inline jboolean j_socket_detail_from_fd(JSocket * socket);

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol)
{
    JSocket socket;
    socket.fd = j_socket(family, type, protocol);
    if (socket.fd < 0) {
        return NULL;
    }
    if (!j_socket_detail_from_fd(&socket)) {
        return NULL;
    }
    return (JSocket *) j_memdup(&socket, sizeof(socket));
}

JSocket *j_socket_new_from_fd(jint fd)
{
    if (fd < 0) {
        return NULL;
    }
    JSocket socket;
    socket.fd = fd;
    if (!j_socket_detail_from_fd(&socket)) {
        return NULL;
    }
    return (JSocket *) j_memdup(&socket, sizeof(socket));
}

void j_socket_close(JSocket * socket)
{
    close(socket->fd);
    j_free(socket);
}

/* 系统调用socket(int,int,int)的包裹函数 */
static inline jint j_socket(jint family, jint type, jint protocol)
{
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

/* 根据文件描述符设置套接字信息 */
static inline jboolean j_socket_detail_from_fd(JSocket * socket)
{
    jint value;
    if (!j_socket_get_option(socket, SOL_SOCKET, SO_TYPE, &value)) {
        return FALSE;
    }
    switch (value) {
    case SOCK_STREAM:
        socket->type = J_SOCKET_TYPE_STREAM;
        break;
    case SOCK_DGRAM:
        socket->type = J_SOCKET_TYPE_DATAGRAM;
        break;
    default:
        socket->type = J_SOCKET_TYPE_INVALID;
    }

    struct sockaddr_storage address;
    juint addrlen = sizeof(address);
    jint fd = socket->fd;
    jint family;

    if (getsockname(fd, (struct sockaddr *) &address, &addrlen) != 0) {
        return FALSE;
    }

    if (addrlen > 0) {
        family = address.ss_family;
    } else {                    /* 在Solaris系统上，如果套接字没有连接，就可能返回addrlen==0，此时通过SO_DOMAIN获取套接字协议族 */
#ifdef SO_DOMAIN
        if (!j_socket_get_option(socket, SOL_SOCKET, SO_DOMAIN, &family)) {
            return FALSE;
        }
#else
        return FALSE;
#endif
    }

    switch (family) {
    case J_SOCKET_FAMILY_INET:
    case J_SOCKET_FAMILY_INET6:
        socket->family = address.ss_family;
        switch (socket->type) {
        case J_SOCKET_TYPE_STREAM:
            socket->protocol = J_SOCKET_PROTOCOL_TCP;
            break;
        case J_SOCKET_TYPE_DATAGRAM:
            socket->protocol = J_SOCKET_PROTOCOL_UDP;
            break;
        case J_SOCKET_TYPE_SEQPACKET:
            socket->protocol = J_SOCKET_PROTOCOL_SCTP;
            break;
        default:
            break;
        }
        break;
    case J_SOCKET_FAMILY_UNIX:
        socket->family = J_SOCKET_FAMILY_UNIX;
        socket->protocol = J_SOCKET_PROTOCOL_DEFAULT;
        break;
    default:
        socket->family = J_SOCKET_FAMILY_INVALID;
        break;
    }

    if (socket->family != J_SOCKET_FAMILY_INVALID) {
        addrlen = sizeof(address);
        if (getpeername(fd, (struct sockaddr *) &address, &addrlen) >= 0) {
            socket->connected = TRUE;
        }
    }
    if (j_socket_get_option(socket, SOL_SOCKET, SO_KEEPALIVE, &value)) {
        socket->keepalive = ! !value;
    } else {
        socket->keepalive = FALSE;
    }

    if (j_socket_get_option(socket, SOL_SOCKET, SO_ACCEPTCONN, &value)) {
        socket->listening = ! !value;
    } else {
        socket->listening = FALSE;
    }

    socket->closed = FALSE;
    socket->blocking = TRUE;
    return TRUE;
}

/* 绑定一个地址 */
jboolean j_socket_bind(JSocket * socket, JSocketAddress * address,
                       jboolean reuse)
{
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    struct sockaddr_storage addr;
    if (!j_socket_address_to_native(address, &addr, sizeof(addr))) {
        return FALSE;
    }
    jboolean so_reuseaddr = ! !reuse;
    j_socket_set_option(socket, SOL_SOCKET, SO_REUSEPORT, so_reuseaddr);
#ifdef SO_REUSEPORT
    jboolean so_reuseport = reuse
        && socket->type == J_SOCKET_TYPE_DATAGRAM;
    j_socket_set_option(socket, SOL_SOCKET, SO_REUSEPORT, so_reuseport);
#endif

    if (bind
        (socket->fd, (struct sockaddr *) &addr,
         j_socket_address_get_native_size(address)) < 0) {
        return FALSE;
    }
    return TRUE;
}

/* 讲套接字设置为被动监听状态 */
jboolean j_socket_listen(JSocket * socket, jint listen_backlog)
{
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    if (listen(socket->fd, listen_backlog) < 0) {
        return FALSE;
    }
    socket->listen_backlog = listen_backlog;
    socket->listening = TRUE;
    return TRUE;
}

/* 连接目标地址，对于面向连接的套接字，这会执行连接。对于无连接的套接字，设置默认的目标地址 */
jboolean j_socket_connect(JSocket * socket, JSocketAddress * address)
{
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    struct sockaddr_storage addr;
    if (!j_socket_address_to_native(address, &addr, sizeof(addr))) {
        return FALSE;
    }

    j_socket_address_init_copy(&socket->remote_address, address);

    while (TRUE) {
        if (connect
            (socket->fd, (struct sockaddr *) &addr,
             j_socket_address_get_native_size(address)) < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EINPROGRESS) {
                if (socket->blocking) {
                    if (j_socket_condition_wait(socket, J_POLL_OUT)) {
                        jint err;
                        if (j_socket_check_connect_result(socket, &err)
                            && !err) {
                            break;
                        }
                    }
                } else {
                    socket->connect_pending = TRUE;
                }
            }
            return FALSE;
        }
    }

    socket->connected = TRUE;
    return TRUE;
}

/* 等待条件condition满足返回TRUE */
jboolean j_socket_condition_wait(JSocket * socket,
                                 JEPollCondition condition)
{
    return j_socket_condition_timed_wait(socket, condition, -1);
}

/* 超时单位，微秒 */
jboolean j_socket_condition_timed_wait(JSocket * socket,
                                       JPollCondition condition,
                                       jint64 timeout)
{
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    if (socket->timeout
        && (timeout < 0 || socket->timeout < timeout / 1000000)) {
        timeout = socket->timeout * 1000;
    } else if (timeout != -1) {
        timeout /= 1000;
    }

    jint start_time = j_get_monotonic_time();

    jint result;
    while (TRUE) {
        jshort revents;
        result = j_poll_simple(socket->fd, condition, timeout, &revents);
        if (result != -1 || errno != EINTR) {
            break;
        }
        if (timeout != -1) {
            timeout -= (j_get_monotonic_time() - start_time) / 1000;
            if (timeout < 0) {
                timeout = 0;
            }
        }
    }
    return result > 0;
}

jboolean j_socket_check_connect_result(JSocket * socket, jint * err)
{
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    if (socket->timed_out) {
        socket->timed_out = FALSE;
        return FALSE;
    }

    jint value;
    if (!j_socket_get_option(socket, SOL_SOCKET, SO_ERROR, &value)) {
        *err = 1;
        return FALSE;
    }
    *err = 0;
    if (value != 0) {
        return FALSE;
    }
    socket->connected = TRUE;
    return TRUE;
}

jboolean j_socket_set_blocking(JSocket * socket, jboolean blocking)
{
    socket->blocking = ! !blocking;
}

jboolean j_socket_get_blocking(JSocket * socket)
{
    return socket->blocking;
}

jboolean j_socket_set_keepalive(JSocket * socket, jboolean keepalive)
{
    if (!j_socket_set_option(socket, SOL_SOCKET, SO_KEEPALIVE, keepalive)) {
        return FALSE;
    }
    socket->keepalive = keepalive;
}

jboolean j_socket_get_keepalive(JSocket * socket)
{
    return socket->keepalive;
}

/*
 * 获取套接字的选项
 * @level API level，如SOL_SOCKET
 * @optname 选项名字，如SO_BROADCAST
 * @value 返回选项值的指针
 */
jboolean j_socket_get_option(JSocket * socket, jint level, jint optname,
                             jint * value)
{
    *value = 0;
    juint size = sizeof(jint);
    if (getsockopt(socket->fd, level, optname, value, &size) != 0) {
        return FALSE;
    }
#if J_BYTE_ORDER == J_BIG_ENDIAN
    /*
     * 如果返回的值不到一个整数的长度，而且系统时大端字节序的，则把返回值移到低位
     * 假设返回的是两个字节的short类型，其值为0x99ff，对于大端字节序的内存如下
     * 高地址 ff990000 低地址  此时，如果调用*((short*)value)取到的是0000，因为value指向起始地址，也就是低地址
     * 因此要将高位右移到低位，0000ff99，此时调用*((short*)value)取到ff99，大端解释为0x99ff
     */
    if (size != sizeof(jint)) {
        *value = *value >> (8 * (sizeof(jint) - size));
    }
#endif
    return TRUE;
}

jboolean j_socket_set_option(JSocket * socket, jint level, jint optname,
                             jint value)
{
    return setsockopt(socket->fd, level, optname, &value,
                      sizeof(jint)) == 0;
}
