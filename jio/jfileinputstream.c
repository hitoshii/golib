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

    JString *buffer;
    jboolean eof;
};

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size);
static jchar *j_file_input_stream_readline(JFileInputStream * stream);
static void j_file_input_stream_close(JFileInputStream * stream);

static JInputStreamInterface j_file_input_stream_interface = {
    (JInputStreamRead) j_file_input_stream_read,
    (JInputStreamReadline) j_file_input_stream_readline,
    (JInputStreamClose) j_file_input_stream_close
};

static inline JFileInputStream *j_file_input_stream_new_from_fd(int fd)
{
    JFileInputStream *stream = j_malloc(sizeof(JFileInputStream));
    stream->fd = fd;
    stream->eof = FALSE;
    stream->buffer = j_string_new();
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
    j_string_free(stream->buffer, TRUE);
    close(stream->fd);
}

static jboolean j_file_input_stream_read_buffer(JFileInputStream * stream)
{
    jchar buf[4096];
    jint n = j_read(stream->fd, buf, sizeof(buf));
    if (n <= 0) {
        stream->eof = TRUE;
        return FALSE;
    }
    j_string_append_len(stream->buffer, buf, n);
    return TRUE;
}

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size)
{
    if (j_string_len(stream->buffer) < size && stream->eof == FALSE) {
        j_file_input_stream_read_buffer(stream);
    }
    size = MIN(j_string_len(stream->buffer), size);
    if (size > 0) {
        memcpy(buffer, j_string_data(stream->buffer), size);
        j_string_erase(stream->buffer, 0, size);
    }
    return size;
}

static jchar *j_file_input_stream_readline(JFileInputStream * stream)
{
    JString *buffer = stream->buffer;
    jchar *newline = strchr(buffer->data, '\n');
    jchar *ret;
    if (newline != NULL) {
        ret = j_strndup(buffer->data, newline - buffer->data);
        j_string_erase(buffer, 0, newline - buffer->data + 1);
        return ret;
    }

    if (j_file_input_stream_read_buffer(stream)) {
        return j_file_input_stream_readline(stream);
    }

    if (j_string_len(stream->buffer) == 0) {
        return NULL;
    }

    ret =
        j_strndup(j_string_data(stream->buffer),
                  j_string_len(stream->buffer));
    j_string_erase(stream->buffer, 0, -1);
    return ret;
}
