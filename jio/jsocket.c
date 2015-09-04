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
#include "jsocket.h"
#include "jpoll.h"
#include <jlib/jxpoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* j_socket_receive_from()的接收者缓存长度 */
#define RECV_ADDR_CACHE_SIZE 8

struct _JSocket {
    JObject parent;
    JSocketFamily family;
    JSocketType type;
    JSocketProtocol protocol;
    int fd;

    int listen_backlog;
    unsigned int timeout;

    unsigned int blocking:1;
    unsigned int keepalive:1;
    unsigned int closed:1;
    unsigned int connected:1;
    unsigned int listening:1;
    unsigned int timed_out:1;
    unsigned int connect_pending:1;

    char *local_address;
    char *remote_address;

    struct {
        JSocketAddress addr;
        struct sockaddr *native;
        int native_len;
        uint64_t last_used;
    } recv_addr_cache[RECV_ADDR_CACHE_SIZE];
};

#define j_socket_check_timeout(socket, val) if(socket->timed_out){socket->timed_out=FALSE;return val;}

/* 根据文件描述符设置套接字信息 */
static inline boolean j_socket_detail_from_fd(JSocket * socket);

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol) {
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

JSocket *j_socket_new_from_fd(int fd) {
    if (fd < 0) {
        return NULL;
    }
    JSocket socket;
    memset(&socket, 0, sizeof(socket));
    socket.fd = fd;
    if (!j_socket_detail_from_fd(&socket)) {
        return NULL;
    }
    return (JSocket *) j_memdup(&socket, sizeof(socket));
}

void j_socket_close(JSocket * socket) {
    if (!socket->closed) {
        close(socket->fd);
        socket->closed = TRUE;
    }
    j_free(socket->local_address);
    j_free(socket->remote_address);
    socket->local_address=NULL;
    socket->remote_address=NULL;
}

/* 根据文件描述符设置套接字信息 */
static inline boolean j_socket_detail_from_fd(JSocket * socket) {
    J_OBJECT_INIT(socket, j_socket_close);
    int value;
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
    unsigned int addrlen = sizeof(address);
    int fd = socket->fd;
    int family;

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

    int flags = fcntl(socket->fd, F_GETFL, 0);
    if (flags >= 0 && flags & O_NONBLOCK) {
        socket->blocking = TRUE;
    }

    return TRUE;
}

/* 绑定一个地址 */
boolean j_socket_bind(JSocket * socket, JSocketAddress * address,
                      boolean reuse) {
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    struct sockaddr_storage addr;
    if (!j_socket_address_to_native(address, &addr, sizeof(addr))) {
        return FALSE;
    }
    boolean so_reuseaddr = ! !reuse;
    j_socket_set_option(socket, SOL_SOCKET, SO_REUSEADDR, so_reuseaddr);
#ifdef SO_REUSEPORT
    boolean so_reuseport = reuse
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
boolean j_socket_listen(JSocket * socket, int listen_backlog) {
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    if (listen(socket->fd, listen_backlog) < 0) {
        return FALSE;
    }
    socket->listen_backlog = listen_backlog;
    socket->listening = TRUE;
    return TRUE;
}

/* 连接目标地址，对于面向连接的套接字，这会执行连接。对于无连接的套接字，设置默认的目标地址 */
boolean j_socket_connect(JSocket * socket, JSocketAddress * address) {
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    struct sockaddr_storage addr;
    if (!j_socket_address_to_native(address, &addr, sizeof(addr))) {
        return FALSE;
    }

    while (j_connect
            (socket->fd, (struct sockaddr *) &addr,
             j_socket_address_get_native_size(address)) < 0) {
        if (errno == EINPROGRESS) {
            if (socket->blocking) {
                if (j_socket_condition_wait(socket, J_POLL_OUT)) {
                    int err;
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

    socket->connected = TRUE;
    return TRUE;
}

/* 接收连接，成功返回新创建得套接字对象，否则返回NULL */
JSocket *j_socket_accept(JSocket * socket) {
    j_return_val_if_fail(socket->closed == FALSE, NULL);
    j_socket_check_timeout(socket, NULL);

    int fd;
    while ((fd = j_accept(socket->fd, NULL, 0)) < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            if (socket->blocking) {
                if (!j_socket_condition_wait(socket, J_POLL_IN)) {
                    return NULL;
                }
                continue;
            }
        }
        return NULL;
    }

    int flags = fcntl(fd, F_GETFD, 0);
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
int j_socket_send_with_blocking(JSocket * socket, const char * buffer,
                                int size, boolean blocking) {
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    if (size < 0) {
        size = j_strlen(buffer);
    }
    int ret;
    while ((ret =
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
    return ret;
}

int j_socket_send(JSocket * socket, const char * buffer, int size) {
    return j_socket_send_with_blocking(socket, buffer, size,
                                       socket->blocking);
}

int j_socket_send_to(JSocket * socket, JSocketAddress * address,
                     const char * buffer, int size) {
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
    int ret;
    while ((ret =
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
    return ret;
}

/* 读取数据 */
int j_socket_receive(JSocket * socket, char * buffer, unsigned int size) {
    return j_socket_receive_with_blocking(socket, buffer, size,
                                          socket->blocking);
}

int j_socket_receive_with_blocking(JSocket * socket, char * buffer,
                                   unsigned int size, boolean blocking) {
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    int ret;
    while ((ret = j_recv(socket->fd, buffer, size, 0)) < 0) {
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
    return ret;
}

int j_socket_receive_from(JSocket * socket, JSocketAddress * address,
                          char * buffer, unsigned int size) {
    if (address == NULL) {
        return j_socket_receive(socket, buffer, size);
    }
    j_return_val_if_fail(socket->closed == FALSE, -1);
    j_socket_check_timeout(socket, -1);

    struct sockaddr_storage addr;
    socklen_t addrlen;

    int ret;
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
boolean j_socket_condition_wait(JSocket * socket,
                                JPollCondition condition) {
    return j_socket_condition_timed_wait(socket, condition, -1);
}

/* 超时单位，微秒 */
boolean j_socket_condition_timed_wait(JSocket * socket,
                                      JPollCondition condition,
                                      int64_t timeout) {
    j_return_val_if_fail(socket->closed == FALSE, FALSE);

    if (socket->timeout
            && (timeout < 0 || socket->timeout < timeout / 1000000)) {
        timeout = socket->timeout * 1000;
    } else if (timeout != -1) {
        timeout /= 1000;
    }

    int start_time = j_get_monotonic_time();

    int result;
    while (TRUE) {
        short revents;
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

boolean j_socket_check_connect_result(JSocket * socket, int * err) {
    j_return_val_if_fail(socket->closed == FALSE, FALSE);
    j_socket_check_timeout(socket, FALSE);

    int value;
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

boolean j_socket_set_blocking(JSocket * socket, boolean blocking) {
    int flags = fcntl(socket->fd, F_GETFL, 0);
    if (flags < 0) {
        return FALSE;
    }

    socket->blocking = ! !blocking;
    int ret;
    if (socket->blocking) {
        flags &= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(socket->fd, F_SETFL, flags) == 0;
}

boolean j_socket_get_blocking(JSocket * socket) {
    return socket->blocking;
}

boolean j_socket_set_keepalive(JSocket * socket, boolean keepalive) {
    if (!j_socket_set_option(socket, SOL_SOCKET, SO_KEEPALIVE, keepalive)) {
        return FALSE;
    }
    socket->keepalive = keepalive;
    return TRUE;
}

boolean j_socket_get_keepalive(JSocket * socket) {
    return socket->keepalive;
}

/*
 * 获取套接字的选项
 * @level API level，如SOL_SOCKET
 * @optname 选项名字，如SO_BROADCAST
 * @value 返回选项值的指针
 */
boolean j_socket_get_option(JSocket * socket, int level, int optname,
                            int * value) {
    *value = 0;
    unsigned int size = sizeof(int);
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
    if (size != sizeof(int)) {
        *value = *value >> (8 * (sizeof(int) - size));
    }
#endif
    return TRUE;
}

boolean j_socket_set_option(JSocket * socket, int level, int optname,
                            int value) {
    return setsockopt(socket->fd, level, optname, &value,
                      sizeof(int)) == 0;
}

boolean j_socket_is_closed(JSocket * socket) {
    return socket->closed;
}

boolean j_socket_is_connected(JSocket * socket) {
    return socket->connected;
}

/* 获取套接字的本地地址，明确绑定的或者连接完成自动生成的 */
boolean j_socket_get_local_address(JSocket * socket,
                                   JSocketAddress * address) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    if (getsockname(socket->fd, (struct sockaddr *) &addr, &addrlen) < 0) {
        return FALSE;
    }
    return j_socket_address_init_from_native(address, &addr, addrlen);
}

/* 获取套接字的远程地址，只对已经连接的套接字有效 */
boolean j_socket_get_remote_address(JSocket * socket,
                                    JSocketAddress * address) {
    if (socket->connect_pending) {
        int err;
        if (!j_socket_check_connect_result(socket, &err)) {
            return FALSE;
        }
        socket->connect_pending = FALSE;
    }
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    if (getpeername(socket->fd, (struct sockaddr *) &addr, &addrlen) < 0) {
        return FALSE;
    }
    return j_socket_address_init_from_native(address, &addr, addrlen);
}

/****************************** 异步操作 ******************************/
typedef struct {
    JSource source;
    JSocket *socket;
    boolean listening;
    short event;
    char *buffer;
    unsigned int size;
} JSocketSource;

static boolean j_socket_source_dispatch(JSource * source,
                                        JSourceFunc callback,
                                        void * user_data) {
    JSocketSource *src = (JSocketSource *) source;
    if (src->event & J_XPOLL_OUT) {
        int ret = j_socket_send_with_blocking(src->socket, src->buffer,
                                              src->size, FALSE);
        ((JSocketSendCallback) callback) (src->socket, ret, user_data);
    } else if (src->event & J_XPOLL_IN) {
        if (!src->listening) {
            int ret =
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

static void j_socket_source_finalize(JSource * source) {
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

static JSocketSource *j_socket_source_new(JSocket * socket, short event) {
    JSocketSource *src =
        (JSocketSource *) j_source_new(&j_socket_source_funcs,
                                       sizeof(JSocketSource));
    j_socket_ref(socket);
    src->socket = socket;
    src->event = event;
    j_source_add_poll_fd((JSource *) src, socket->fd, event);
    return src;
}

void j_socket_send_async(JSocket * socket, const char * buffer, int size,
                         JSocketSendCallback callback, void * user_data) {
    j_return_if_fail(socket->closed == FALSE);
    if (size < 0) {
        size = j_strlen(buffer);
    }
    JSocketSource *src = j_socket_source_new(socket, J_XPOLL_OUT);
    src->buffer = j_memdup(buffer, size);
    src->size = size;
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}

void j_socket_receive_async(JSocket * socket, JSocketRecvCallback callback,
                            void * user_data) {
    j_socket_receive_with_length_async(socket, 4096, callback, user_data);
}

/* 尽可能读取长度为length的数据 */
void j_socket_receive_with_length_async(JSocket * socket, unsigned int length,
                                        JSocketRecvCallback callback,
                                        void * user_data) {
    j_return_if_fail(socket->closed == FALSE);
    JSocketSource *src =
        (JSocketSource *) j_socket_source_new(socket, J_XPOLL_IN);
    src->size = length;
    src->buffer = j_malloc(sizeof(char) * src->size);
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}

void j_socket_accept_async(JSocket * socket,
                           JSocketAcceptCallback callback,
                           void * user_data) {
    j_return_if_fail(socket->closed == FALSE);
    JSocketSource *src =
        (JSocketSource *) j_socket_source_new(socket, J_XPOLL_IN);
    src->listening = TRUE;
    j_source_set_callback((JSource *) src, (JSourceFunc) callback,
                          user_data, NULL);
    j_source_attach((JSource *) src, NULL);
    j_source_unref((JSource *) src);
}


const char *j_socket_get_remote_address_string(JSocket *socket) {
    if(socket->remote_address==NULL) {
        JSocketAddress address;
        if(j_socket_get_local_address(socket, &address)) {
            socket->remote_address=j_inet_socket_address_to_string(&address);
        }
    }
    return socket->remote_address;
}
const char *j_socket_get_local_address_string(JSocket *socket) {
    if(socket->local_address==NULL) {
        JSocketAddress address;
        if(j_socket_get_local_address(socket, &address)) {
            socket->local_address=j_inet_socket_address_to_string(&address);
        }
    }
    return socket->local_address;
}
