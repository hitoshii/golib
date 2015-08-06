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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "jfileinputstream.h"
#include <jlib/jlib.h>
#include <fcntl.h>
#include <string.h>

struct _JFileInputStream {
    JInputStream parent;
    jint fd;
};

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size);
static void j_file_input_stream_close(JFileInputStream * stream);

static JInputStreamInterface j_file_input_stream_interface = {
    (JInputStreamRead) j_file_input_stream_read,
    (JInputStreamClose) j_file_input_stream_close
};

static inline JFileInputStream *j_file_input_stream_new_from_fd(int fd)
{
    JFileInputStream *stream = j_malloc(sizeof(JFileInputStream));
    stream->fd = fd;
    j_input_stream_init((JInputStream *) stream,
                        &j_file_input_stream_interface);
    return stream;
}

/* 打开文件读，失败返回NULL */
JFileInputStream *j_file_read(JFile * f)
{
    jint fd = j_file_open_fd(f, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    return j_file_input_stream_new_from_fd(fd);
}

JFileInputStream *j_file_input_stream_open(const jchar * path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    return j_file_input_stream_new_from_fd(fd);
}

static void j_file_input_stream_close(JFileInputStream * stream)
{
    close(stream->fd);
}

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size)
{
    return j_read(stream->fd, buffer, size);
}
