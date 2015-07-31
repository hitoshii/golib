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
#ifndef __JIO_OUTPUT_STREAM_H__
#define __JIO_OUTPUT_STREAM_H__

#include <jlib/jtypes.h>
#include <jlib/jobject.h>


typedef struct _JOutputStreamInterface JOutputStreamInterface;

typedef struct {
    JObject parent;
    JOutputStreamInterface *iface;
    jboolean closed;
} JOutputStream;

typedef jint(*JOutputStreamWrite) (JOutputStream * stream,
                                   const jchar * buf, jint len);
typedef void (*JOutputStreamClose) (JOutputStream * stream);

struct _JOutputStreamInterface {
    JOutputStreamWrite write;
    JOutputStreamClose close;
};

void j_output_stream_init(JOutputStream * stream,
                          JOutputStreamInterface * interface);

jint j_output_stream_write(JOutputStream * stream, const jchar * buf,
                           jint len);
void j_output_stream_close(JOutputStream * stream);

#endif
