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
#ifndef __JIO_INPUT_STREAM_H__
#define __JIO_INPUT_STREAM_H__

#include <jlib/jtypes.h>
#include <jlib/jobject.h>

typedef struct _JInputStreamInterface JInputStreamInterface;

typedef struct {
    JObject parent;
    JInputStreamInterface *iface;
    boolean closed;
} JInputStream;

typedef int(*JInputStreamRead) (JInputStream * stream, void *buffer,
                                unsigned int size);
/*
 * 读取一行，如果失败或者到文件结束，返回NULL
 * 读取成功返回读取到的一行数据，不包括换行符，需要释放返回的字符串
 */
// typedef char *(*JInputStreamReadline) (JInputStream * stream);
/*
 * 关闭流
 */
typedef void (*JInputStreamClose) (JInputStream * stream);

struct _JInputStreamInterface {
    JInputStreamRead read;
    // JInputStreamReadline readline;
    JInputStreamClose close;
};

void j_input_stream_init(JInputStream * stream,
                         JInputStreamInterface * interface);

boolean j_input_stream_is_closed(JInputStream * input_stream);

int j_input_stream_read(JInputStream * stream, void *buffer, unsigned int size);
// char *j_input_stream_readline(JInputStream * stream);
void j_input_stream_close(JInputStream * stream);

#endif
