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

#ifndef __JIO_MAIN_LOOP_H__
#define __JIO_MAIN_LOOP_H__

#include "jsocket.h"

typedef struct _JMainLoop JMainLoop;


/*
 * Creates a new main loop
 */
JMainLoop *j_main_loop_new(void);

/*
 * Gets the default main loop
 */
JMainLoop *j_main_loop_default(void);

/*
 * Runs the main loop
 */
void j_main_loop_run(JMainLoop * loop);


void j_main_loop_quit(JMainLoop * loop);

void j_main_loop_free(JMainLoop * loop);

#define J_SOCKET_EVENT_ACCEPT 0x01
#define J_SOCKET_EVENT_SEND   0x02
#define J_SOCKET_EVENT_RECV   0x04
typedef void (*JMainLoopForeach) (JSocket * sock, unsigned int events,
                                  void *user_data);
/*
 * 遍历所有的socket
 */
void j_main_loop_foreach_socket(JMainLoop * loop, JMainLoopForeach foreach,
                                void *user_data);

/*
 * Runs the default main loop
 */
void j_main(void);
/*
 * Quits the default main loop
 */
void j_main_quit(void);


/*
 * Accepts a connection asynchronously
 */

/*
 * @listen the listening socket
 * @client the new connection accepted, or NULL on error
 * @user_data the custom data
 *
 * returns 0 to stop accepting connection
 */
typedef int (*JSocketAcceptNotify) (JSocket * listen,
                                    JSocket * client, void *user_data);
void j_socket_accept_async(JSocket * sock, JSocketAcceptNotify notify,
                           void *user_data);

/*
 * Sends data asynchronously
 */

/*
 * @sock the socket that sends data
 * @data the data to send
 * @count the total length in byte of data
 * @n the length in byte of data that is send
 * @user_data the custom data
 *
 */
typedef void (*JSocketSendNotify) (JSocket * sock, const void *data,
                                   unsigned int count, unsigned int n,
                                   void *user_data);
void j_socket_send_async(JSocket * sock, JSocketSendNotify notify,
                         const void *data, unsigned int count,
                         void *user_data);


/*
 * Receives data asynchronously
 */
typedef void (*JSocketRecvNotify) (JSocket * sock,
                                   const char *data,
                                   unsigned int len,
                                   JSocketRecvResultType type,
                                   void *user_data);
void j_socket_recv_async(JSocket * sock, JSocketRecvNotify notify,
                         void *user_data);
void j_socket_recv_len_async(JSocket * sock, JSocketRecvNotify notify,
                             unsigned int len, void *user_data);

#endif
