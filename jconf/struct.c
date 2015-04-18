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
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "struct.h"

static inline JConfNode *j_conf_node_alloc(JConfNodeType type);

JConfNode *j_conf_node_create_object(void)
{
    JConfNode *node = j_conf_node_alloc(J_CONF_NODE_OBJECT);
    node->data.children = NULL;
    return node;
}

JConfNode *j_conf_node_create_array(void)
{
    JConfNode *node = j_conf_node_alloc(J_CONF_NODE_ARRAY);
    node->data.children = NULL;
    return node;
}

JConfNode *j_conf_node_create_int(int64_t integer)
{
    JConfNode *node = j_conf_node_alloc(J_CONF_NODE_INT);
    node->data.integer = integer;
    return node;
}

JConfNode *j_conf_node_create_float(double floatig)
{
    JConfNode *node = j_conf_node_alloc(J_CONF_NODE_FLOAT);
    node->data.floating = floatig;
    return node;
}

JConfNode *j_conf_node_create_string(const char *string)
{
    return j_conf_node_create_string_take(j_strdup(string));
}

JConfNode *j_conf_node_create_string_take(char *string)
{
    JConfNode *node = j_conf_node_alloc(J_CONF_NODE_STRING);
    node->data.string = string;
    return node;
}

JConfNode *j_conf_node_create_true(void)
{
    return j_conf_node_alloc(J_CONF_NODE_TRUE);
}

JConfNode *j_conf_node_create_false(void)
{
    return j_conf_node_alloc(J_CONF_NODE_FALSE);
}

JConfNode *j_conf_node_create_null(void)
{
    return j_conf_node_alloc(J_CONF_NODE_NULL);
}

void j_conf_node_set_name(JConfNode * node, const char *name)
{
    j_conf_node_set_name_take(node, j_strdup(name));
}

void j_conf_node_set_name_take(JConfNode * node, char *name)
{
    j_free(node->name);
    node->name = name;
}

void j_conf_node_free(JConfNode * node)
{
    if (j_conf_node_is_array(node) || j_conf_node_is_object(node)) {
        j_list_free_full(node->data.children,
                         (JListDestroy) j_conf_node_free);
    } else if (j_conf_node_is_string(node)) {
        j_free(node->data.string);
    }
    j_free(node->name);
    j_free(node);
}


static inline JConfNode *j_conf_node_alloc(JConfNodeType type)
{
    JConfNode *node = (JConfNode *) j_malloc(sizeof(JConfNode));
    node->type = type;
    node->name = NULL;
    return node;
}


JConfRoot *j_conf_root_new(const char *name)
{
    JConfRoot *root = (JConfRoot *) j_malloc(sizeof(JConfRoot));
    root->name = j_strdup(name);
    root->children = NULL;
    return root;
}

void j_conf_root_free(JConfRoot * root)
{
    j_free(root->name);
    j_list_free_full(root->children, (JListDestroy) j_conf_node_free);
}
