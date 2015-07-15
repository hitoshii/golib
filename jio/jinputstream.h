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
#ifndef __JIO_INPUT_STREAM_H__
#define __JIO_INPUT_STREAM_H__

#include <jlib/jtypes.h>
#include <jlib/jobject.h>

// typedef struct _JInputStream JInputStream;

typedef jint(*JInputStreamRead) (jpointer priv, void *buffer, juint size);
typedef void (*JInputStreamClose) (jpointer priv);

typedef struct {
    JInputStreamRead read;
    JInputStreamClose close;
} JInputStreamInterface;

typedef JObject JInputStream;

JInputStream *j_input_stream_new_proxy(jpointer priv,
                                       JInputStreamInterface * interface);

// void j_input_stream_ref(JInputStream * stream);
// void j_input_stream_unref(JInputStream * stream);

jint j_input_stream_read(JInputStream * stream, void *buffer, juint size);
void j_input_stream_close(JInputStream * stream);

#endif
