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
#ifndef __JIO_IO_SOURCE_H__
#define __JIO_IO_SOURCE_H__

#include <jlib/jlib.h>


typedef struct _JIOSource JIOSource;

typedef void (*JIOSendCallback) (JObject * socket, int ret, void * user_data);
typedef boolean(*JIORecvCallback) (JObject * socket, const char * buffer, int size,void * user_data);

/** j_io_source_new:
 * @fd: 文件描述符
 * @event: 监听的事件
 * @object: 拥有该文件描述符的对象
 */
JIOSource *j_io_source_new(int fd, short event, JObject *object);

#endif
