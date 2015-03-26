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
#include "jmainloop.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <jlib/jlib.h>
#include <errno.h>


struct _JSocket {
    int fd;

    char *sockname;
    char *peername;

    void *data;
};

void *j_socket_get_data(JSocket * sock)
{
    return sock->data;
}

void j_socket_set_data(JSocket * sock, void *data)
{
    sock->data = data;
}


static inline JSocket *j_socket_alloc(int fd)
{
    JSocket *jsock = (JSocket *) j_malloc(sizeof(JSocket));
    jsock->fd = fd;
    jsock->sockname = NULL;
    jsock->peername = NULL;
    return jsock;
}

/*
 * Returns the UNIX file descriptor
 */
int j_socket_get_fd(JSocket * jsock)
{
    return jsock->fd;
}

const char *j_socket_get_peer_name(JSocket * jsock)
{
    if (jsock->peername) {
        return jsock->peername;
    }
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buf[32];
    if (getpeername(j_socket_get_fd(jsock),
                    (struct sockaddr *) &addr, &addrlen) != 0 ||
        inet_ntop(AF_INET, (const void *) &addr.sin_addr.s_addr, buf,
                  sizeof(buf) / sizeof(char)) == NULL) {
        return NULL;
    }
    jsock->peername = j_strdup_printf("%s:%u", buf, ntohs(addr.sin_port));
    return jsock->peername;
}

const char *j_socket_get_socket_name(JSocket * jsock)
{
    if (jsock->sockname) {
        return jsock->sockname;
    }
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buf[32];
    if (getsockname(j_socket_get_fd(jsock),
                    (struct sockaddr *) &addr, &addrlen) != 0 ||
        inet_ntop(AF_INET, (const void *) &addr.sin_addr.s_addr, buf,
                  sizeof(buf) / sizeof(char)) == NULL) {
        return NULL;
    }
    jsock->sockname = j_strdup_printf("%s:%u", buf, ntohs(addr.sin_port));
    return jsock->sockname;
}

/*
 * Creates a negative socket which listens on port
 */
JSocket *j_socket_listen_on(unsigned short port, unsigned int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return NULL;
    }
    /*
     * Re-use address
     */
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr))) {
        close(fd);
        return NULL;
    }
    if (listen(fd, backlog)) {
        close(fd);
        return NULL;
    }
    return j_socket_alloc(fd);
}

JSocket *j_socket_accept(JSocket * jsock)
{
    int fd = j_socket_get_fd(jsock);

    int accfd = accept(fd, NULL, NULL);
    if (accfd < 0) {
        return NULL;
    }

    return j_socket_alloc(accfd);
}

JSocket *j_socket_connect_to(const char *server, const char *service)
{
    JSocket *jsock = j_socket_new();
    if (jsock == NULL) {
        return NULL;
    }
    if (!j_socket_connect(jsock, server, service)) {
        j_socket_close(jsock);
        return NULL;
    }
    return jsock;
}

/*
 * Creates a client socket
 */
JSocket *j_socket_new(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return NULL;
    }
    return j_socket_alloc(fd);
}

/*
 * Connects to a server, server may be ip address or a domain
 * jsock must be a client socket
 */
int j_socket_connect(JSocket * jsock, const char *server,
                     const char *service)
{
    int fd = j_socket_get_fd(jsock);

    struct addrinfo *ailist, *aip;
    struct addrinfo hint;

    hint.ai_flags = AI_CANONNAME;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;
    hint.ai_addrlen = 0;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if (getaddrinfo(server, service, &hint, &ailist) != 0) {
        return 0;
    }
    for (aip = ailist; aip != NULL; aip = aip->ai_next) {
        if (connect(fd, aip->ai_addr, sizeof(struct sockaddr_in)) == 0) {
            freeaddrinfo(ailist);
            return 1;
        }
    }
    freeaddrinfo(ailist);
    return 0;
}

/*
 * Closes a socket and frees all associated memory
 */
void j_socket_close(JSocket * jsock)
{
    close(jsock->fd);
    j_free(jsock->peername);
    j_free(jsock->sockname);
    j_free(jsock);
}

/*
 * Makes the socket work in block/nonblock mode
 */
int j_socket_set_blocking(JSocket * jsock, int block)
{
    int fd = j_socket_get_fd(jsock);
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return 0;
    }
    if (!block) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(fd, F_SETFL, flags) == 0;
}

/*
 * Sends data
 * Returns the length of data that is send
 */

static inline int j_socket_send_with_flags(JSocket * jsock,
                                           const void *data,
                                           unsigned int count, int flags)
{
    int fd = j_socket_get_fd(jsock);
    int n = send(fd, data, count, flags);
    return n;
}

int j_socket_send(JSocket * jsock, const void *data, unsigned int count)
{
    return j_socket_send_with_flags(jsock, data, count, 0);
}

int j_socket_send_dontwait(JSocket * jsock, const void *data,
                           unsigned int count)
{
    return j_socket_send_with_flags(jsock, data, count, MSG_DONTWAIT);
}


/********************** JSocketRecvResult ***********************/

static inline JSocketRecvResult *j_socket_recv_result_new(void *data,
                                                          unsigned int len,
                                                          JSocketRecvResultType
                                                          type)
{
    JSocketRecvResult *res =
        (JSocketRecvResult *) j_malloc(sizeof(JSocketRecvResult));
    res->data = data;
    res->len = len;
    res->type = type;
    return res;
}

static inline JSocketRecvResult
    * j_socket_recv_result_new_from_bytes(JByteArray * array,
                                          JSocketRecvResultType type)
{
    unsigned int len = j_byte_array_get_len(array);
    void *data = j_byte_array_free(array, 0);
    return j_socket_recv_result_new(data, len, type);
}

void j_socket_recv_result_free(JSocketRecvResult * res)
{
    j_free(res->data);
    j_free(res);
}


/**************************************************************/


static inline JSocketRecvResult *j_socket_recv_with_flags(JSocket * sock,
                                                          unsigned int len,
                                                          int flags)
{
    int fd = j_socket_get_fd(sock);
    JByteArray *array = j_byte_array_new();
    JSocketRecvResultType type = J_SOCKET_RECV_NORMAL;

    if (len == 0) {
        len = (unsigned int) -1;
    }

    char buf[4096];
    while (len > 0) {
        unsigned int count = len > 4096 ? 4096 : len;
        int n = recv(fd, buf, count, flags);
        if (n <= 0) {
            if (n == 0) {
                type = J_SOCKET_RECV_EOF;
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                type = J_SOCKET_RECV_ERR;
            }
            break;
        }
        len -= n;
        j_byte_array_append(array, buf, n);
    }


    JSocketRecvResult *res =
        j_socket_recv_result_new_from_bytes(array, type);

    return res;
}

/*
 * Receives data
 */
JSocketRecvResult *j_socket_recv(JSocket * jsock, unsigned int len)
{
    return j_socket_recv_with_flags(jsock, len, 0);
}

JSocketRecvResult *j_socket_recv_dontwait(JSocket * jsock,
                                          unsigned int len)
{
    return j_socket_recv_with_flags(jsock, len, MSG_DONTWAIT);
}

/*
 * data必须为四个字节
 * 解析四个字节的二进制数组
 * 返回长度
 */
static inline unsigned int parse_length(const char *data);

/*
 * 生成四个字节的数组，表示长度len
 */
static inline const void *generate_length(unsigned int len);

/* 中间数据 */
typedef struct {
    void *notify;
    void *user_data;
    unsigned int len;
} JSocketPackageData;

static inline JSocketPackageData
    * j_socket_package_data_new(void *notify, void *user_data)
{
    JSocketPackageData *data =
        (JSocketPackageData *) j_malloc(sizeof(JSocketPackageData));
    data->notify = notify;
    data->user_data = user_data;
    return data;
}

static inline void j_socket_package_data_free(JSocketPackageData * data)
{
    j_free(data);
}


/*
 * 获取消息内容的回调函数
 */
static void recv_package_callback(JSocket * sock, const char *buf,
                                  unsigned int count,
                                  JSocketRecvResultType type, void *udata)
{
    JSocketPackageData *data = (JSocketPackageData *) udata;
    JSocketRecvPackageNotify notify =
        (JSocketRecvPackageNotify) data->notify;
    void *user_data = data->user_data;
    unsigned int len = data->len;

    if (buf == NULL || type == J_SOCKET_RECV_ERR) {
        notify(sock, NULL, 0, J_SOCKET_RECV_ERR, user_data);
    } else if (count != len) {
        notify(sock, NULL, 0, type, user_data);
    } else {
        notify(sock, buf, len, type, user_data);
    }
    j_socket_package_data_free(data);
}

/*
 * 获取消息长度的回调函数
 */
static void recv_package_len_callback(JSocket * sock,
                                      const char *buf,
                                      unsigned int count,
                                      JSocketRecvResultType type,
                                      void *udata)
{
    JSocketPackageData *data = (JSocketPackageData *) udata;
    JSocketRecvPackageNotify notify =
        (JSocketRecvPackageNotify) data->notify;
    void *user_data = data->user_data;
    if (type == J_SOCKET_RECV_ERR || buf == NULL) {
        notify(sock, NULL, 0, J_SOCKET_RECV_ERR, user_data);
        j_socket_package_data_free(data);
        return;
    }
    unsigned int len = parse_length((const char *) buf);
    if (count != 4 || type != J_SOCKET_RECV_NORMAL || len == 0) {
        notify(sock, NULL, 0, type, user_data);
        j_socket_package_data_free(data);
        return;
    }
    data->len = len;
    j_socket_recv_len_async(sock, recv_package_callback, len, data);
}

void j_socket_recv_package(JSocket * sock, JSocketRecvPackageNotify notify,
                           void *user_data)
{
    JSocketPackageData *data =
        j_socket_package_data_new(notify, user_data);
    j_socket_recv_len_async(sock, recv_package_len_callback, 4, data);
}


/********************** 发送消息 ************************************/

static void send_package_callback(JSocket * sock, const void *data,
                                  unsigned int count, unsigned int n,
                                  void *udata)
{
    JSocketPackageData *pdata = (JSocketPackageData *) udata;
    JSocketSendPackageNotify notify =
        (JSocketSendPackageNotify) pdata->notify;
    void *user_data = pdata->user_data;

    notify(sock, data, count, n, user_data);

    j_socket_package_data_free(pdata);
}

void j_socket_send_package(JSocket * sock, JSocketSendPackageNotify notify,
                           const void *data, unsigned int len,
                           void *user_data)
{
    if (len == 0) {
        return;
    }
    JSocketPackageData *pdata =
        j_socket_package_data_new(notify, user_data);
    const void *buf = generate_length(len);
    JByteArray *array = j_byte_array_new();
    j_byte_array_append(array, buf, 4);
    j_byte_array_append(array, data, len);
    j_socket_send_async(sock, send_package_callback,
                        j_byte_array_get_data(array),
                        j_byte_array_get_len(array), pdata);
    j_byte_array_free(array, 1);
}


static inline unsigned int parse_length(const char *data)
{
    unsigned int sum = 0;
    sum += data[0] << 24;
    sum += data[1] << 16;
    sum += data[2] << 8;
    sum += data[3];
    return sum;
}

static inline const void *generate_length(unsigned int len)
{
    static char buf[4];
    buf[0] = len / (1 << 24);
    buf[1] = len % (1 << 24) / (1 << 16);
    buf[2] = len % (1 << 16) / (1 << 8);
    buf[3] = len % (1 << 8);
    return (const void *) buf;
}
