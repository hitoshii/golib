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
#ifndef __JIO_SOCKET_H__
#define __JIO_SOCKET_H__

#include <jlib/jlib.h>
#include "jsocketaddress.h"

typedef struct _JSocket JSocket;
typedef struct _JSocketSource JSocketSource;


JSocket *j_socket_source_get_socket(JSocketSource *src);

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol);
JSocket *j_socket_new_from_fd(int fd);

/* XXX 该函数会直接关闭套接字，并释放所有资源，不管引用计数，尽量用j_socket_unref */
void j_socket_close(JSocket * socket);

#define j_socket_ref(s) J_OBJECT_REF(s)
#define j_socket_unref(s) J_OBJECT_UNREF(s)

/* 绑定一个地址 */
boolean j_socket_bind(JSocket * socket, JSocketAddress * address,
                      boolean reuse);
/* 讲套接字设置为被动监听状态，需要线绑定一个地址 */
boolean j_socket_listen(JSocket * socket, int listen_backlog);
/* 连接目标地址，对于面向连接的套接字，这会执行连接。对于无连接的套接字，设置默认的目标地址 */
boolean j_socket_connect(JSocket * socket, JSocketAddress * address);

/* 接收连接，成功返回新创建得套接字对象，否则返回NULL */
JSocket *j_socket_accept(JSocket * socket);
typedef boolean(*JSocketAcceptCallback) (JSocket * socket, JSocket * conn,
        void * user_data);
void j_socket_accept_async(JSocket * socket,
                           JSocketAcceptCallback callback,
                           void * user_data);

/* 发送数据 */
int j_socket_send_with_blocking(JSocket * socket, const void * buffer,
                                unsigned int size, boolean blocking);
int j_socket_send(JSocket * socket, const void * buffer, unsigned int size);
/* 如果address为NULL，则等同于 j_socket_send() */
int j_socket_send_to(JSocket * socket, JSocketAddress * address,
                     const void * buffer, unsigned int size);

typedef void (*JSocketSendCallback) (JSocket * socket, int ret,
                                     void * user_data);
void j_socket_send_async(JSocket * socket, const void * buffer, unsigned int size,
                         JSocketSendCallback callback, void * user_data);

/* 读取数据 */
int j_socket_receive(JSocket * socket, char * buffer, unsigned int size);
int j_socket_receive_with_blocking(JSocket * socket, char * buffer,
                                   unsigned int size, boolean blocking);
/* 如果address为NULL，则等同于j_socket_receive() */
int j_socket_receive_from(JSocket * socket, JSocketAddress * address,
                          char * buffer, unsigned int size);
typedef boolean(*JSocketRecvCallback) (JSocket * socket,
                                       const void * buffer, int size,
                                       void * user_data);
void j_socket_receive_async(JSocket * socket, JSocketRecvCallback callback,
                            void * user_data);
/* 尽可能读取长度为length的数据 */
void j_socket_receive_with_length_async(JSocket * socket, unsigned int length,
                                        JSocketRecvCallback callback,
                                        void * user_data);

/* 等待条件condition满足返回TRUE */
boolean j_socket_condition_wait(JSocket * socket,
                                JPollCondition condition);
boolean j_socket_condition_timed_wait(JSocket * socket,
                                      JPollCondition condition,
                                      int64_t timeout);

/* 判断非阻塞连接操作是否成功 */
boolean j_socket_check_connect_result(JSocket * socket, int * err);


boolean j_socket_set_blocking(JSocket * socket, boolean blocking);
boolean j_socket_get_blocking(JSocket * socket);
boolean j_socket_set_keepalive(JSocket * socket, boolean keepalive);
boolean j_socket_get_keepalive(JSocket * socket);
/*
 * 获取套接字的选项
 * @level API level，如SOL_SOCKET
 * @optname 选项名字，如SO_BROADCAST
 * @value 返回选项值的指针
 */
boolean j_socket_get_option(JSocket * socket, int level, int optname,
                            int * value);
boolean j_socket_set_option(JSocket * socket, int level, int optname,
                            int value);


boolean j_socket_is_closed(JSocket * socket);
boolean j_socket_is_connected(JSocket * socket);

/* 获取套接字的本地地址，明确绑定的或者连接完成自动生成的 */
boolean j_socket_get_local_address(JSocket * socket,
                                   JSocketAddress * address);
/* 获取套接字的远程地址，只对已经连接的套接字有效 */
boolean j_socket_get_remote_address(JSocket * socket,
                                    JSocketAddress * address);

const char *j_socket_get_remote_address_string(JSocket *socket);
const char *j_socket_get_local_address_string(JSocket *socket);

#endif
