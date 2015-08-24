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
#include "jbufferedinputstream.h"
#include <jlib/jlib.h>
#include <string.h>

struct _JBufferedInputStream {
    JInputStream parent;
    JInputStream *base_stream;
    // JString *buffer;
    juint8 *buffer;
    juint buffer_len;
    juint buffer_total;
    juint buffer_start;
    jboolean eof;
};
#define DEFAULT_BUFFER_SIZE (1024)


static jint j_buffered_input_stream_read(JBufferedInputStream * stream,
        void *buffer, juint size);
static void j_buffered_input_stream_close(JBufferedInputStream *
        buffered_stream);

static JInputStreamInterface j_buffered_input_stream_interface = {
    (JInputStreamRead) j_buffered_input_stream_read,
    (JInputStreamClose) j_buffered_input_stream_close
};

JBufferedInputStream *j_buffered_input_stream_new(JInputStream *
        input_stream) {
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
    buffered_stream->buffer=(juint8*)j_malloc(sizeof(juint8)*DEFAULT_BUFFER_SIZE);
    buffered_stream->buffer_len=0;
    buffered_stream->buffer_start=0;
    buffered_stream->buffer_total=DEFAULT_BUFFER_SIZE;
    return buffered_stream;
}

static jboolean j_buffered_input_stream_read_buffer(JBufferedInputStream *
        stream) {
    if(stream->buffer_start>0) {
        memmove(stream->buffer, stream->buffer+stream->buffer_start, stream->buffer_start);
        stream->buffer_start=0;
    }
    juint8 buf[4096];
    jint n = j_input_stream_read(stream->base_stream, buf, sizeof(buf));
    if (n <= 0) {
        stream->eof = TRUE;
        return FALSE;
    }
    while(stream->buffer_len+n>=stream->buffer_total) {
        stream->buffer_total<<=1;
        stream->buffer=(juint8*)j_realloc(stream->buffer, stream->buffer_total);
    }
    memcpy(stream->buffer+stream->buffer_start+stream->buffer_len, buf, n);
    stream->buffer_len+=n;
    return TRUE;
}

static jint j_buffered_input_stream_read(JBufferedInputStream * stream,
        void *buffer, juint size) {
    if (J_UNLIKELY(j_input_stream_is_closed((JInputStream *) stream))) {
        return -1;
    }
    while (stream->buffer_len < size && stream->eof == FALSE) {
        j_buffered_input_stream_read_buffer(stream);
    }
    size = MIN(stream->buffer_len, size);
    if (size > 0) {
        memcpy(buffer, stream->buffer+stream->buffer_start, size);
        stream->buffer_start+=size;
        stream->buffer_len-=size;
    }
    return size;
}

juint8 *j_buffered_input_stream_find_newline(JBufferedInputStream *stream) {
    jint i, len=stream->buffer_start+stream->buffer_len;
    for (i = stream->buffer_start; i < len; i++) {
        juint8 c=stream->buffer[i];
        if(c=='\n') {
            return stream->buffer+i;
        }
    }
    return NULL;
}

jchar *j_buffered_input_stream_readline(JBufferedInputStream *
                                        stream) {
    if (J_UNLIKELY
            (j_input_stream_is_closed((JInputStream *) stream))) {
        return NULL;
    }

    juint8 *newline = j_buffered_input_stream_find_newline(stream);
    jchar *ret;
    if (newline != NULL) {
        juint len=newline - stream->buffer - stream->buffer_start;
        ret = j_strndup((jchar*)(stream->buffer+stream->buffer_start), len);
        len+=1; /* 去除换行符号 */
        stream->buffer_start+=len;
        stream->buffer_len-=len;
        return ret;
    }

    if (j_buffered_input_stream_read_buffer(stream)) {
        return j_buffered_input_stream_readline(stream);
    }

    if (stream->buffer_len == 0) {
        return NULL;
    }

    ret = j_strndup((jchar*)(stream->buffer+stream->buffer_start), stream->buffer_len);
    stream->buffer_start+=stream->buffer_len;
    stream->buffer_len=0;
    return ret;
}

jint j_buffered_input_stream_get(JBufferedInputStream *stream) {
    if (J_UNLIKELY
            (j_input_stream_is_closed((JInputStream *) stream))) {
        return -1;
    }
    while (stream->buffer_len < 1 && stream->eof == FALSE) {
        j_buffered_input_stream_read_buffer(stream);
    }
    if(stream->buffer_len<=0) {
        return -1;
    }
    jint c=*(stream->buffer+stream->buffer_start);
    stream->buffer_start++;
    stream->buffer_len--;
    return c;
}

static void j_buffered_input_stream_close(JBufferedInputStream *
        buffered_stream) {
    j_free(buffered_stream->buffer);
    buffered_stream->buffer = NULL;
    J_OBJECT_UNREF(buffered_stream->base_stream);
}

void j_buffered_input_stream_push(JBufferedInputStream * stream,
                                  const jchar * buf, jint size) {
    if (J_UNLIKELY
            (j_input_stream_is_closed((JInputStream *) stream) || size == 0)) {
        return;
    }
    if (size < 0) {
        size = j_strlen(buf);
    }
    if(stream->buffer_start<size) {
        while(size+stream->buffer_len>stream->buffer_total) {
            stream->buffer_total<<=1;
            stream->buffer=j_realloc(stream->buffer, stream->buffer_total);
        }
        memmove(stream->buffer+size, stream->buffer+stream->buffer_start, stream->buffer_len);
        stream->buffer_start=size;
    }
    memcpy(stream->buffer+stream->buffer_start-size, buf, size);
    stream->buffer_start-=size;
    stream->buffer_len+=size;
}

void j_buffered_input_stream_push_line(JBufferedInputStream * stream,
                                       const jchar * buf, jint size) {
    j_buffered_input_stream_push_c(stream, '\n');
    j_buffered_input_stream_push(stream, buf, size);
}

void j_buffered_input_stream_push_c(JBufferedInputStream * stream, jchar c) {
    j_buffered_input_stream_push(stream, &c, 1);
}
