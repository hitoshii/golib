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
#ifndef __JIO_SOCKET_H__
#define __JIO_SOCKET_H__

#include <jlib/jlib.h>
#include "jsocketaddress.h"

typedef struct _JSocket JSocket;

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol);
JSocket *j_socket_new_from_fd(jint fd);

/* XXX 该函数会直接关闭套接字，并释放所有资源，不管引用计数 */
void j_socket_close(JSocket * socket);

void j_socket_ref(JSocket * socket);
void j_socket_unref(JSocket * socket);

/* 绑定一个地址 */
jboolean j_socket_bind(JSocket * socket, JSocketAddress * address,
                       jboolean reuse);
/* 讲套接字设置为被动监听状态，需要线绑定一个地址 */
jboolean j_socket_listen(JSocket * socket, jint listen_backlog);
/* 连接目标地址，对于面向连接的套接字，这会执行连接。对于无连接的套接字，设置默认的目标地址 */
jboolean j_socket_connect(JSocket * socket, JSocketAddress * address);

/* 接收连接，成功返回新创建得套接字对象，否则返回NULL */
JSocket *j_socket_accept(JSocket * socket);
typedef jboolean(*JSocketAcceptCallback) (JSocket * socket, JSocket * conn,
                                          jpointer user_data);
void j_socket_accept_async(JSocket * socket,
                           JSocketAcceptCallback callback,
                           jpointer user_data);

/* 发送数据 */
jint j_socket_send_with_blocking(JSocket * socket, const jchar * buffer,
                                 jint size, jboolean blocking);
jint j_socket_send(JSocket * socket, const jchar * buffer, jint size);

typedef void (*JSocketSendCallback) (JSocket * socket, jint ret,
                                     jpointer user_data);
void j_socket_send_async(JSocket * socket, const jchar * buffer, jint size,
                         JSocketSendCallback callback, jpointer user_data);

/* 读取数据 */
jint j_socket_receive(JSocket * socket, jchar * buffer, juint size);
jint j_socket_receive_with_blocking(JSocket * socket, jchar * buffer,
                                    juint size, jboolean blocking);
typedef jboolean(*JSocketRecvCallback) (JSocket * socket,
                                        const jchar * buffer, jint size,
                                        jpointer user_data);
void j_socket_receive_async(JSocket * socket, JSocketRecvCallback callback,
                            jpointer user_data);
/* 尽可能读取长度为length的数据 */
void j_socket_receive_with_length_async(JSocket * socket, juint length,
                                        JSocketRecvCallback callback,
                                        jpointer user_data);

/* 等待条件condition满足返回TRUE */
jboolean j_socket_condition_wait(JSocket * socket,
                                 JPollCondition condition);
jboolean j_socket_condition_timed_wait(JSocket * socket,
                                       JPollCondition condition,
                                       jint64 timeout);

/* 判断非阻塞连接操作是否成功 */
jboolean j_socket_check_connect_result(JSocket * socket, jint * err);


jboolean j_socket_set_blocking(JSocket * socket, jboolean blocking);
jboolean j_socket_get_blocking(JSocket * socket);
jboolean j_socket_set_keepalive(JSocket * socket, jboolean keepalive);
jboolean j_socket_get_keepalive(JSocket * socket);
/*
 * 获取套接字的选项
 * @level API level，如SOL_SOCKET
 * @optname 选项名字，如SO_BROADCAST
 * @value 返回选项值的指针
 */
jboolean j_socket_get_option(JSocket * socket, jint level, jint optname,
                             jint * value);
jboolean j_socket_set_option(JSocket * socket, jint level, jint optname,
                             jint value);


jboolean j_socket_is_closed(JSocket * socket);
jboolean j_socket_is_connected(JSocket * socket);

#endif
