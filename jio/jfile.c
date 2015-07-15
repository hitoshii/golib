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
#include "jfile.h"
#include <jlib/jlib.h>

typedef struct {
    jchar *path;
} JFilePriv;

static void j_file_priv_free(JFilePriv * priv)
{
    j_free(priv->path);
    j_free(priv);
}

/* 该函数不会失败，除非内存分配出错 */
JFile *j_file_new(const jchar * path)
{
    JFilePriv *priv = j_malloc(sizeof(JFilePriv));
    priv->path = j_strdup(path);

    return j_object_new_proxy(priv, (JObjectDestroy) j_file_priv_free);
}

jint j_file_open_fd(JFile * f, jint mode)
{
    JFilePriv *priv = j_object_get_priv(f);
    return open(priv->path, mode);
}

/* 获取目录，new的时候指定的目录 */
const jchar *j_file_get_path(JFile * f)
{
    JFilePriv *priv = j_object_get_priv(f);
    return priv->path;
}
