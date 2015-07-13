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

    jint ref;                   /* 引用计数 */
};

#define j_socket_check_timeout(socket, val) if(socket->timed_out){socket->timed_out=FALSE;return val;}

/* 系统调用的包裹函数 */
static inline jint j_socket(jint family, jint type, jint protocol);
static inline jint j_send(jint sockfd, const void *buf, size_t len,
                          int flags);
static inline jint j_sendto(jint sockfd, const void *buf, size_t len,
                            int flags, const struct sockaddr *dest_addr,
                            socklen_t addrlen);
static inline jint j_recv(int sockfd, void *buf, size_t len, int flags);
static inline jint j_recvfrom(int sockfd, void *buf, size_t len, int flags,
                              struct sockaddr *src_addr,
                              socklen_t * addrlen);
/* 根据文件描述符设置套接字信息 */
static inline jboolean j_socket_detail_from_fd(JSocket * socket);

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol)
{
    JSocket socket;
    bzero(&socket, sizeof(socket));
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

static inline jint j_send(jint sockfd, const void *buf, size_t len,
                          int flags)
{
    jint ret;
    while ((ret = send(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    return ret;
}

static inline jint j_sendto(jint sockfd, const void *buf, size_t len,
                            int flags, const struct sockaddr *dest_addr,
                            socklen_t addrlen)
{
    jint ret;
    while ((ret = sendto(sockfd, buf, len, flags, dest_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    return ret;
}

static inline jint j_recv(int sockfd, void *buf, size_t len, int flags)
{
    jint ret;
    while ((ret = recv(sockfd, buf, len, flags)) < 0) {
        if (errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    return ret;
}

static inline jint j_recvfrom(int sockfd, void *buf, size_t len, int flags,
                              struct sockaddr *src_addr,
                              socklen_t * addrlen)
{
    jint ret;
    while ((ret =
            recvfrom(sockfd, buf, len, flags, src_addr, addrlen)) < 0) {
        if (errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    return ret;
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

    jint flags = fcntl(socket->fd, F_GETFL, 0);
    if (flags >= 0 && flags & O_NONBLOCK) {
        socket->blocking = TRUE;
    }

    socket->ref = 1;
    return TRUE;
}


void j_socket_ref(JSocket * socket)
{
    j_atomic_int_inc(&socket->ref);
}

void j_socket_unref(JSocket * socket)
{
    if (j_atomic_int_dec_and_test(&socket->ref)) {
        j_socket_close(socket);
    }
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
        break;
    }

    socket->connected = TRUE;
    return TRUE;
}

/* 接收连接，成功返回新创建得套接字对象，否则返回NULL */
JSocket *j_socket_accept(JSocket * socket)
{
    j_return_val_if_fail(socket->closed == FALSE, NULL);
    j_socket_check_timeout(socket, NULL);

    jint fd;
    while (TRUE) {
        if ((fd = accept(socket->fd, NULL, 0)) < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (socket->blocking) {
                    if (!j_socket_condition_wait(socket, J_POLL_IN)) {
                        return NULL;
                    }
                    continue;
                }
            }
            return NULL;
        }
        break;
    }

    jint flags = fcntl(fd, F_GETFD, 0);
    if (flags >= 0 && (flags & FD_CLOEXEC) == 0) {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }

    JSocket *new = j_socket_new_from_fd(fd);
    if (new == NULL) {
        close(fd);
    } else {
        new->protocol = socket->protocol;
    }
    return new;
}

#ifdef MSG_NOSIGNAL             /* 不会产生SIGPIPE信号，该信号会终止程序 */
#define J_SOCKET_DEFAULT_SEND_FLAGS MSG_NOSIGNAL
#else
#define J_SOCKET_DEFAULT_SEND_FLAGS 0
#endif

/* 发送数据 */
jint j_socket_send_with_blocking(JSocket * socket, const jchar * buffer,
                                 jint size, jboolean blocking)
{
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    if (size < 0) {
        size = j_strlen(buffer);
    }
    jint ret;
    while (TRUE) {
        if ((ret =
             j_send(socket->fd, buffer, size,
                    J_SOCKET_DEFAULT_SEND_FLAGS)) < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (blocking) {
                    if (!j_socket_condition_wait(socket, J_POLL_OUT)) {
                        return -1;
                    }
                    continue;
                }
            }
            return -1;
        }
        break;
    }
    return ret;
}

jint j_socket_send(JSocket * socket, const jchar * buffer, jint size)
{
    return j_socket_send_with_blocking(socket, buffer, size,
                                       socket->blocking);
}

jint j_socket_send_to(JSocket * socket, JSocketAddress * address,
                      const jchar * buffer, jint size)
{
    if (address == NULL) {
        return j_socket_send(socket, buffer, size);
    }

    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    if (size < 0) {
        size = j_strlen(buffer);
    }
    struct sockaddr_storage addr;
    socklen_t addrlen = j_socket_address_get_native_size(address);
    j_socket_address_to_native(address, &addr, sizeof(addr));
    jint ret;
    while (TRUE) {
        if ((ret =
             j_sendto(socket->fd, buffer, size,
                      J_SOCKET_DEFAULT_SEND_FLAGS,
                      (struct sockaddr *) &addr, addrlen)) < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (socket->blocking) {
                    if (!j_socket_condition_wait(socket, J_POLL_OUT)) {
                        return -1;
                    }
                    continue;
                }
            }
            return -1;
        }
        break;
    }
    return ret;
}

/* 读取数据 */
jint j_socket_receive(JSocket * socket, jchar * buffer, juint size)
{
    return j_socket_receive_with_blocking(socket, buffer, size,
                                          socket->blocking);
}

jint j_socket_receive_with_blocking(JSocket * socket, jchar * buffer,
                                    juint size, jboolean blocking)
{
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    jint ret;
    while (TRUE) {
        if ((ret = j_recv(socket->fd, buffer, size, 0)) < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (blocking) {
                    if (!j_socket_condition_wait(socket, J_POLL_IN)) {
                        return -1;
                    }
                    continue;
                }
            }
            return -1;
        }
        break;
    }
    return ret;
}

jint j_socket_receive_from(JSocket * socket, JSocketAddress * address,
                           jchar * buffer, juint size)
{
    if (address == NULL) {
        return j_socket_receive(socket, buffer, size);
    }
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    struct sockaddr_storage addr;
    socklen_t addrlen;

    jint ret;
    while (TRUE) {
        if ((ret =
             j_recvfrom(socket->fd, buffer, size, 0,
                        (struct sockaddr *) &addr, &addrlen)) < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (socket->blocking) {
                    if (!j_socket_condition_wait(socket, J_POLL_IN)) {
                        return -1;
                    }
                    continue;
                }
            }
            return -1;
        }
        break;
    }
    j_socket_address_init_from_native(address, &addr, addrlen);
    return ret;
}

/* 等待条件condition满足返回TRUE */
jboolean j_socket_condition_wait(JSocket * socket,
                                 JPollCondition condition)
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
    j_socket_check_timeout(socket, FALSE);

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
    jint flags = fcntl(socket->fd, F_GETFL, 0);
    if (flags < 0) {
        return FALSE;
    }

    socket->blocking = ! !blocking;
    jint ret;
    if (socket->blocking) {
        flags &= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(socket->fd, F_SETFL, flags) == 0;
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
    return TRUE;
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

jboolean j_socket_is_closed(JSocket * socket)
{
    return socket->closed;
}

jboolean j_socket_is_connected(JSocket * socket)
{
    return socket->connected;
}

/****************************** 异步操作 ******************************/
typedef struct {
    JSource source;
    JSocket *socket;
    jboolean listening;
    jshort event;
    jchar *buffer;
    juint size;
} JSocketSource;

static jboolean j_socket_source_dispatch(JSource * source,
                                         JSourceFunc callback,
                                         jpointer user_data)
{
    JSocketSource *src = (JSocketSource *) source;
    if (src->event & J_EPOLL_OUT) {
        jint ret = j_socket_send_with_blocking(src->socket, src->buffer,
                                               src->size, FALSE);
        ((JSocketSendCallback) callback) (src->socket, ret, user_data);
    } else if (src->event & J_EPOLL_IN) {
        if (!src->listening) {
            jint ret =
                j_socket_receive_with_blocking(src->socket, src->buffer,
                                               src->size, FALSE);
            return ((JSocketRecvCallback) callback) (src->socket,
                                                     src->buffer, ret,
                                                     user_data);
        } else {
            JSocket *new = j_socket_accept(src->socket);
            return ((JSocketAcceptCallback) callback) (src->socket, new,
                                                       user_data);
        }
    }
    return FALSE;
}

static void j_socket_source_finalize(JSource * source)
{
    JSocketSource *src = (JSocketSource *) source;
    j_socket_unref(src->socket);
    j_free(src->buffer);
}

JSourceFuncs j_socket_source_funcs = {
    NULL,
    NULL,
    j_socket_source_dispatch,
    j_socket_source_finalize
};

static JSocketSource *j_socket_source_new(JSocket * socket, jshort event)
{
    JSocketSource *src =
        (JSocketSource *) j_source_new(&j_socket_source_funcs,
                                       sizeof(JSocketSource));
    j_socket_ref(socket);
    src->socket = socket;
    src->event = event;
    j_source_add_poll_fd((JSource *) src, socket->fd, event);
    return src;
}

void j_socket_send_async(JSocket * socket, const jchar * buffer, jint size,
                         JSocketSendCallback callback, jpointer user_data)
{
    j_return_if_fail(socket->closed == FALSE);
    if (size < 0) {
        size = j_strlen(buffer);
    }
    JSocketSource *src = j_socket_source_new(socket, J_EPOLL_OUT);
    src->buffer = j_memdup(buffer, size);
    src->size = size;
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}

void j_socket_receive_async(JSocket * socket, JSocketRecvCallback callback,
                            jpointer user_data)
{
    j_socket_receive_with_length_async(socket, 4096, callback, user_data);
}

/* 尽可能读取长度为length的数据 */
void j_socket_receive_with_length_async(JSocket * socket, juint length,
                                        JSocketRecvCallback callback,
                                        jpointer user_data)
{
    j_return_if_fail(socket->closed == FALSE);
    JSocketSource *src =
        (JSocketSource *) j_socket_source_new(socket, J_EPOLL_IN);
    src->size = length;
    src->buffer = j_malloc(sizeof(jchar) * src->size);
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}

void j_socket_accept_async(JSocket * socket,
                           JSocketAcceptCallback callback,
                           jpointer user_data)
{
    j_return_if_fail(socket->closed == FALSE);
    JSocketSource *src =
        (JSocketSource *) j_socket_source_new(socket, J_EPOLL_IN);
    src->listening = TRUE;
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}
