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
#include "jconfloader.h"

struct _JConfLoader {
    JObject parent;
    JFileInputStream *input_stream;
    JConfRoot *root;
};

static void j_conf_loader_free(JConfLoader * loader);

JConfLoader *j_conf_loader_new(JFile * f)
{
    JFileInputStream *input_stream = j_file_read(f);
    if (input_stream == NULL) {
        return NULL;
    }
    JConfLoader *loader = (JConfLoader *) j_malloc(sizeof(JConfLoader));
    J_OBJECT_INIT(loader, j_conf_loader_free);
    loader->input_stream = input_stream;
    loader->root = j_conf_root_new();
    return loader;
}

static void j_conf_loader_free(JConfLoader * loader)
{
    j_file_input_stream_unref(loader->input_stream);
    j_conf_root_unref(loader->root);
}
