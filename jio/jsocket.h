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
#ifndef __J_SOCKET_H__
#define __J_SOCKET_H__

#include <jlib/jlib.h>

typedef struct _JSocket JSocket;



void *j_socket_get_data(JSocket * sock);
void j_socket_set_data(JSocket * sock, void *data);

/*
 * Returns the UNIX file descriptor
 */
int j_socket_get_fd(JSocket * jsock);


/* getpeername */
const char *j_socket_get_peer_name(JSocket * jsock);
/* getsockname */
const char *j_socket_get_socket_name(JSocket * jsock);

/*
 * Creates a negative/server socket which listens on port
 */
JSocket *j_socket_listen_on(unsigned short port, unsigned int backlog);

JSocket *j_socket_accept(JSocket * jsock);


JSocket *j_socket_connect_to(const char *server, const char *service);

/*
 * Creates a client socket
 */
JSocket *j_socket_new(void);

/*
 * Connects to a server, server may be ip address or a domain
 * jsock must be a client socket
 */
int j_socket_connect(JSocket * jsock, const char *server,
                     const char *service);


/*
 * Closes a socket and frees all associated memory
 */
void j_socket_close(JSocket * jsock);

/*
 * Makes the socket work in block/nonblock mode
 */
int j_socket_set_blocking(JSocket * jsock, int block);


/*
 * Sends data
 * Returns the length of data that is send
 */
int j_socket_send(JSocket * jsock, const void *data, unsigned int count);
int j_socket_send_dontwait(JSocket * jsock, const void *data,
                           unsigned int count);

/*
 * Receives data
 */

typedef enum {
    J_SOCKET_RECV_NORMAL,
    J_SOCKET_RECV_EOF,
    J_SOCKET_RECV_ERR,
} JSocketRecvResultType;

typedef struct {
    void *data;
    unsigned int len;
    JSocketRecvResultType type;
} JSocketRecvResult;
#define j_socket_recv_result_get_data(res)  (res)->data
#define j_socket_recv_result_get_len(res)   (res)->len
#define j_socket_recv_result_get_type(res)  (res)->type
#define j_socket_recv_result_is_normal(res) \
            (j_socket_recv_result_get_type(res)==J_SOCKET_RECV_NORMAL)
#define j_socket_recv_result_is_eof(res) \
            (j_socket_recv_result_get_type(res)==J_SOCKET_RECV_EOF)
#define j_socket_recv_result_is_error(res) \
            (j_socket_recv_result_get_type(res)==J_SOCKET_RECV_ERR)

void j_socket_recv_result_free(JSocketRecvResult * res);

JSocketRecvResult *j_socket_recv(JSocket * jsock, unsigned int len);
JSocketRecvResult *j_socket_recv_dontwait(JSocket * jsock,
                                          unsigned int len);

void j_socket_set_recv_result(JSocket * jsock);
void j_socket_remove_recv_result(JSocket * jsock);


#endif
