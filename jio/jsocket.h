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


typedef struct _JSocket JSocket;


typedef enum {
    J_SOCKET_TYPE_SERVER,
    J_SOCKET_TYPE_NEW,
    J_SOCKET_TYPE_CONNECTED,
} JSocketType;

JSocketType j_socket_get_type(JSocket * jsock);

/*
 * Returns the UNIX file descriptor
 */
int j_socket_get_fd(JSocket * jsock);
int j_socket_get_extra(JSocket * jsock);
void j_socket_set_extra(JSocket * jsock, int data);


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


#endif
