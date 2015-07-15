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

typedef struct {
    jint fd;
} JFileInputStreamPriv;

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size);
static void j_file_input_stream_close(JFileInputStream * stream);

static JInputStreamInterface j_file_input_stream_interface = {
    j_file_input_stream_read,
    j_file_input_stream_close
};

/* 打开文件读，失败返回NULL */
JFileInputStream *j_file_read(JFile * f)
{
    jint fd = j_file_open_fd(f, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    JFileInputStreamPriv *priv = j_malloc(sizeof(JFileInputStreamPriv));
    priv->fd = fd;
    JFileInputStream *stream =
        j_input_stream_new_proxy(priv, &j_file_input_stream_interface);
    return stream;
}

static void j_file_input_stream_close(JFileInputStream * stream)
{
    JFileInputStreamPriv *priv = (JFileInputStreamPriv *) stream->priv;
    close(priv->fd);
    j_free(priv);
    j_free(stream);
}

static jint j_file_input_stream_read(JFileInputStream * stream,
                                     void *buffer, juint size)
{
    JFileInputStreamPriv *priv = (JFileInputStreamPriv *) stream->priv;
    return j_read(priv->fd, buffer, size);
}
