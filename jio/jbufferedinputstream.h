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
#ifndef __JIO_BUFFERED_INPUT_STREAM_H__
#define __JIO_BUFFERED_INPUT_STREAM_H__

#include "jinputstream.h"

typedef struct _JBufferedInputStream JBufferedInputStream;

JBufferedInputStream *j_buffered_input_stream_new(JInputStream *
        input_stream);

#define j_buffered_input_stream_ref(s) J_OBJECT_REF(s)
#define j_buffered_input_stream_unref(s) J_OBJECT_UNREF(s)

jchar *j_buffered_input_stream_readline(JBufferedInputStream *
                                        buffered_stream);
void j_buffered_input_stream_push(JBufferedInputStream * stream,
                                  const jchar * buf, jint size);
void j_buffered_input_stream_push_line(JBufferedInputStream * stream,
                                       const jchar * buf, jint size);
void j_buffered_input_stream_push_c(JBufferedInputStream * stream,
                                    jchar c);

/**
 * j_buffered_input_stream_get:
 * @buffered_stream: JBufferedInputStream
 *
 * Read a byte from buffer
 * Returns: negative if EOF
 */
jint j_buffered_input_stream_get(JBufferedInputStream *buffered_stream);

#endif
