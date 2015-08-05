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
#include "jconfroot.h"
#include <jlib/jlib.h>

struct _JConfRoot {
    JObject parent;
    JHashTable *nodes;
};

static void j_conf_root_free(JConfRoot * root);

JConfRoot *j_conf_root_new(void)
{
    JConfRoot *root = (JConfRoot *) j_malloc(sizeof(JConfRoot));
    J_OBJECT_INIT(root, j_conf_root_free);
    root->nodes =
        j_hash_table_new(10, j_str_hash, j_str_equal,
                         (JKeyDestroyFunc) j_free,
                         (JValueDestroyFunc) j_object_unref);
    return root;
}


static void j_conf_root_free(JConfRoot * root)
{
    j_hash_table_free_full(root->nodes);
}

void j_conf_root_set(JConfRoot * root, const jchar * name,
                     JConfNode * node)
{
    j_conf_root_set_take(root, j_strdup(name), node);
}

void j_conf_root_set_take(JConfRoot * root, jchar * name, JConfNode * node)
{
    j_hash_table_insert(root->nodes, name, node);
}

JConfNode *j_conf_root_get(JConfRoot * root, const jchar * name)
{
    return (JConfNode *) j_hash_table_find(root->nodes, name);
}

void j_conf_root_set_string(JConfRoot * root, const jchar * name,
                            const jchar * string)
{
    j_conf_root_set(root, name,
                    j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}

void j_conf_root_set_float(JConfRoot * root, const jchar * name,
                           jdouble floating)
{
    j_conf_root_set(root, name,
                    j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

void j_conf_root_set_bool(JConfRoot * root, const jchar * name, jboolean b)
{
    j_conf_root_set(root, name, j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

void j_conf_root_set_null(JConfRoot * root, const jchar * name)
{
    j_conf_root_set(root, name, j_conf_node_new(J_CONF_NODE_TYPE_NULL));
}
