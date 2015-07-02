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
void j_socket_close(JSocket * socket);

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

#endif
