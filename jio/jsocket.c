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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <jlib/jlib.h>


struct _JSocket {
    int fd;
    JSocketType type;

    char *sockname;
    char *peername;

    int extra;
};

int j_socket_get_extra(JSocket * jsock)
{
    return jsock->extra;
}

void j_socket_set_extra(JSocket * jsock, int data)
{
    jsock->extra = data;
}

static inline JSocket *j_socket_alloc(int fd, JSocketType type)
{
    JSocket *jsock = (JSocket *) j_malloc(sizeof(JSocket));
    jsock->fd = fd;
    jsock->type = type;
    jsock->sockname = NULL;
    jsock->peername = NULL;
    return jsock;
}

JSocketType j_socket_get_type(JSocket * jsock)
{
    return jsock->type;
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
    return j_socket_alloc(fd, J_SOCKET_TYPE_SERVER);
}

JSocket *j_socket_accept(JSocket * jsock)
{
    int fd = j_socket_get_fd(jsock);

    int accfd = accept(fd, NULL, NULL);
    if (accfd < 0) {
        return NULL;
    }

    return j_socket_alloc(accfd, J_SOCKET_TYPE_CONNECTED);
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
    return j_socket_alloc(fd, J_SOCKET_TYPE_NEW);
}

/*
 * Connects to a server, server may be ip address or a domain
 * jsock must be a client socket
 */
int j_socket_connect(JSocket * jsock, const char *server,
                     const char *service)
{
    if (j_socket_get_type(jsock) != J_SOCKET_TYPE_NEW) {
        return 0;
    }
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
            jsock->type = J_SOCKET_TYPE_CONNECTED;
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
            type = (n == 0) ? J_SOCKET_RECV_EOF : J_SOCKET_RECV_ERR;
            break;
        }
        len -= n;
        j_byte_array_append(array, buf, n);
    }

    return j_socket_recv_result_new_from_bytes(array, type);
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
