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
    JHashTable *env;
    JConfRoot *root;
};

static void j_conf_loader_free(JConfLoader * loader);

JConfLoader *j_conf_loader_new(void)
{
    JConfLoader *loader = (JConfLoader *) j_malloc(sizeof(JConfLoader));
    J_OBJECT_INIT(loader, j_conf_loader_free);
    loader->root = j_conf_root_new();
    loader->env = j_hash_table_new(5, j_str_hash, j_str_equal,
                                   (JKeyDestroyFunc) j_free,
                                   (JValueDestroyFunc) j_object_unref);
    return loader;
}

static void j_conf_loader_free(JConfLoader * loader)
{
    j_hash_table_free_full(loader->env);
    j_conf_root_unref(loader->root);
}

JConfRoot *j_conf_loader_get_root(JConfLoader * loader)
{
    return loader->root;
}

static inline void j_conf_loader_put(JConfLoader * loader,
                                     const jchar * name, JConfNode * node)
{
    j_hash_table_insert(loader->env, j_strdup(name), node);
}

void j_conf_loader_put_integer(JConfLoader * loader, const jchar * name,
                               jint64 integer)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, integer));
}

void j_conf_loader_put_string(JConfLoader * loader, const jchar * name,
                              const jchar * string)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}

void j_conf_loader_put_float(JConfLoader * loader, const jchar * name,
                             jdouble floating)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

void j_conf_loader_put_bool(JConfLoader * loader, const jchar * name,
                            jboolean b)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

void j_conf_loader_put_null(JConfLoader * loader, const jchar * name)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_NULL));
}

typedef enum {
    J_CONF_LOADER_START,
} JConfLoaderState;

#define j_conf_is_delimiter(c) ((c)<=32)

jboolean j_conf_loader_loads(const jchar * path)
{
    JFileInputStream *input_stream = j_file_input_stream_open(path);
    if (input_stream == NULL) {
        return FALSE;
    }
    JConfLoaderState state = J_CONF_LOADER_START;
    jchar buf[1024];
    jint i, n;
    while ((n =
            j_input_stream_read((JInputStream *) input_stream, buf,
                                sizeof(buf))) > 0) {
        for (i = 0; i < n; i++) {

        }
    }
    j_file_input_stream_unref(input_stream);
    return FALSE;
}
