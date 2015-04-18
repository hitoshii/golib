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
#include <string.h>

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
    if (node == NULL) {
        return;
    }
    if (j_conf_node_is_array(node) || j_conf_node_is_object(node)) {
        j_list_free_full(node->data.children,
                         (JListDestroy) j_conf_node_free);
    } else if (j_conf_node_is_string(node)) {
        j_free(node->data.string);
    }
    j_free(node->name);
    j_free(node);
}

void j_conf_node_append_child(JConfNode * node, JConfNode * child)
{
    if (!j_conf_node_is_array(node) && !j_conf_node_is_object(node)) {
        return;
    }
    node->data.children = j_list_append(node->data.children, child);
}

JList *j_conf_node_get_children(JConfNode * node)
{
    if (!j_conf_node_is_array(node) && !j_conf_node_is_object(node)) {
        return NULL;
    }
    return node->data.children;
}


static inline JConfNode *j_conf_node_alloc(JConfNodeType type)
{
    JConfNode *node = (JConfNode *) j_malloc(sizeof(JConfNode));
    node->type = type;
    node->name = NULL;
    return node;
}

/*
 * 获取最后一个名为name的节点
 */
JConfNode *j_conf_object_get(JConfNode * obj, const char *name)
{
    JConfNode *child = NULL;
    JList *children = j_conf_node_get_children(obj);
    while (children) {
        JConfNode *node = (JConfNode *) j_list_data(children);
        if (strcmp(name, j_conf_node_get_name(node)) == 0) {
            child = node;
        }
        children = j_list_next(children);
    }
    return child;
}

int64_t j_conf_int_get(JConfNode * node)
{
    if (!j_conf_node_is_int(node)) {
        return 0;
    }
    return node->data.integer;
}

double j_conf_float_get(JConfNode * node)
{
    if (!j_conf_node_is_float(node)) {
        return 0.0;
    }
    return node->data.floating;
}

const char *j_conf_string_get(JConfNode * node)
{
    if (!j_conf_node_is_string(node)) {
        return NULL;
    }
    return node->data.string;
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
    j_free(root);
}

void j_conf_root_append(JConfRoot * root, JConfNode * node)
{
    root->children = j_list_append(root->children, node);
}
