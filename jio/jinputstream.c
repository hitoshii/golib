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

typedef struct {
    JInputStreamInterface *interface;
    jpointer priv;
    jboolean closed;
} JInputStreamPriv;

static void j_input_stream_free(JInputStreamPriv * priv)
{
    if (priv->closed == FALSE && priv->interface->close) {
        priv->interface->close(priv->priv);
        priv->closed = TRUE;
    }
    j_free(priv);
}

JInputStream *j_input_stream_new_proxy(jpointer priv,
                                       JInputStreamInterface * interface)
{
    JInputStreamPriv *p = j_malloc(sizeof(JInputStreamPriv));
    p->priv = priv;
    p->interface = interface;
    p->closed = FALSE;
    JInputStream *stream = (JInputStream *) j_object_new_proxy(p,
                                                               (JObjectDestroy)
                                                               j_input_stream_free);
    return stream;
}


jint j_input_stream_read(JInputStream * stream, void *buffer, juint size)
{
    JInputStreamPriv *priv = j_object_get_priv(stream);
    if (priv->interface->read) {
        return priv->interface->read(priv->priv, buffer, size);
    }
    return -1;
}

void j_input_stream_close(JInputStream * stream)
{
    JInputStreamPriv *priv = j_object_get_priv(stream);
    if (priv->closed == FALSE && priv->interface->close) {
        priv->interface->close(priv->priv);
        priv->closed = TRUE;
    }
}
