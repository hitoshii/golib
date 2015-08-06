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

#endif
