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
#include "jinputstream.h"
#include <jlib/jlib.h>

JInputStream *j_input_stream_new_proxy(jpointer priv,
                                       JInputStreamInterface * interface)
{
    JInputStream *stream = j_malloc(sizeof(JInputStream));
    stream->ref = 1;
    stream->priv = priv;
    stream->interface = interface;
    return stream;
}

void j_input_stream_ref(JInputStream * stream)
{
    j_atomic_int_inc(&stream->ref);
}

void j_input_stream_unref(JInputStream * stream)
{
    if (j_atomic_int_dec_and_test(&stream->ref)) {
        j_input_stream_close(stream);
    }
}

jint j_input_stream_read(JInputStream * stream, void *buffer, juint size)
{
    if (stream->interface->read) {
        return stream->interface->read(stream, buffer, size);
    }
    return -1;
}

void j_input_stream_close(JInputStream * stream)
{
    if (stream->interface->close) {
        stream->interface->close(stream);
    }
}
