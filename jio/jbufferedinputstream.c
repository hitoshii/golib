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
#include "jbufferedinputstream.h"
#include <jlib/jlib.h>
#include <string.h>

struct _JBufferedInputStream {
    JInputStream parent;
    JInputStream *base_stream;
    JString *buffer;
    jboolean eof;
};


static jint j_buffered_input_stream_read(JBufferedInputStream * stream,
                                         void *buffer, juint size);
static void j_buffered_input_stream_close(JBufferedInputStream *
                                          buffered_stream);

static JInputStreamInterface j_buffered_input_stream_interface = {
    (JInputStreamRead) j_buffered_input_stream_read,
    (JInputStreamClose) j_buffered_input_stream_close
};

JBufferedInputStream *j_buffered_input_stream_new(JInputStream *
                                                  input_stream)
{
    if (J_UNLIKELY(j_input_stream_is_closed(input_stream))) {
        return NULL;
    }
    JBufferedInputStream *buffered_stream =
        (JBufferedInputStream *) j_malloc(sizeof(JBufferedInputStream));
    j_input_stream_init((JInputStream *) buffered_stream,
                        &j_buffered_input_stream_interface);
    J_OBJECT_REF(input_stream);
    buffered_stream->base_stream = input_stream;
    buffered_stream->eof = FALSE;
    buffered_stream->buffer = j_string_new();
    return buffered_stream;
}

static jboolean j_buffered_input_stream_read_buffer(JBufferedInputStream *
                                                    stream)
{
    jchar buf[4096];
    jint n = j_input_stream_read(stream->base_stream, buf, sizeof(buf));
    if (n <= 0) {
        stream->eof = TRUE;
        return FALSE;
    }
    j_string_append_len(stream->buffer, buf, n);
    return TRUE;
}

static jint j_buffered_input_stream_read(JBufferedInputStream * stream,
                                         void *buffer, juint size)
{
    if (J_UNLIKELY(j_input_stream_is_closed((JInputStream *) stream))) {
        return -1;
    }
    if (j_string_len(stream->buffer) < size && stream->eof == FALSE) {
        j_buffered_input_stream_read_buffer(stream);
    }
    size = MIN(j_string_len(stream->buffer), size);
    if (size > 0) {
        memcpy(buffer, j_string_data(stream->buffer), size);
        j_string_erase(stream->buffer, 0, size);
    }
    return size;
}

jchar *j_buffered_input_stream_readline(JBufferedInputStream *
                                        buffered_stream)
{
    if (J_UNLIKELY
        (j_input_stream_is_closed((JInputStream *) buffered_stream))) {
        return NULL;
    }
    JString *buffer = buffered_stream->buffer;
    jchar *newline = strchr(buffer->data, '\n');
    jchar *ret;
    if (newline != NULL) {
        ret = j_strndup(buffer->data, newline - buffer->data);
        j_string_erase(buffer, 0, newline - buffer->data + 1);
        return ret;
    }

    if (j_buffered_input_stream_read_buffer(buffered_stream)) {
        return j_buffered_input_stream_readline(buffered_stream);
    }

    if (j_string_len(buffered_stream->buffer) == 0) {
        return NULL;
    }

    ret =
        j_strndup(j_string_data(buffered_stream->buffer),
                  j_string_len(buffered_stream->buffer));
    j_string_erase(buffered_stream->buffer, 0, -1);
    return ret;
}

static void j_buffered_input_stream_close(JBufferedInputStream *
                                          buffered_stream)
{
    j_string_free(buffered_stream->buffer, TRUE);
    buffered_stream->buffer = NULL;
    J_OBJECT_UNREF(buffered_stream->base_stream);
}
