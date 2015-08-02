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
#include "jconfnode.h"
#include <stdarg.h>

/**
 * SECTION: JConfNode
 * @title: Node of JConf
 * @short_description: a node presents one type of data
 * @include: jconfnode.h
 *
 */

/**
 * JConfNodeType:
 * @JCONF_NODE_TYPE_NULL: NULL
 * @JCONF_NODE_TYPE_STRING: jchar*
 * @JCONF_NODE_TYPE_INTEGER: jint64
 * @JCONF_NODE_TYPE_FLOAT: jdouble
 * @JCONF_NODE_TYPE_BOOL: jboolean
 * @JCONF_NODE_TYPE_OBJECT: JConfNode*
 * @JCONF_NODE_TYPE_ARRAY: JPtrArray<JConfNode*>
 */

struct _JConfNode {
    JObject parent;
    jchar *name;
    JConfNodeType type;
    union {
        jchar *data_string;
        jint64 data_integer;
        jdouble data_floating;
        JConfNode *data_object;
        JPtrArray *data_array;
    } data;
};
#define d_string data.data_string
#define d_integer data.data_integer
#define d_floating data.data_floating
#define d_object data.data_object
#define d_array data.data_array

/**
 * j_conf_node_get_type:
 * @node:
 *
 * Returns: the type of JConfNode
 */
JConfNodeType j_conf_node_get_type(JConfNode * node)
{
    return node->type;
}

/**
 * j_conf_node_get_name:
 * @node:
 *
 * name of a JConfNode, also known as key
 *
 * Returns: the name of JConfNode
 */
const jchar *j_conf_node_get_name(JConfNode * node)
{
    return node->name;
}

static inline JConfNode *j_conf_node_alloc(const jchar * name,
                                           JConfNodeType type, ...);
static inline JPtrArray *j_conf_node_ptr_array_new();
static void j_conf_node_free(JConfNode * node);



static inline JConfNode *j_conf_node_alloc(const jchar * name,
                                           JConfNodeType type, ...)
{
    JConfNode *node = (JConfNode *) j_malloc(sizeof(JConfNode));
    J_OBJECT_INIT(node, j_conf_node_free);
    va_list ap;
    va_start(ap, type);

    node->name = j_strdup(name);
    node->type = type;
    switch (type) {
    case JCONF_NODE_TYPE_STRING:
        node->d_string = j_strdup(va_arg(ap, jchar *));
        break;
    case JCONF_NODE_TYPE_INTEGER:
        node->d_integer = va_arg(ap, jint64);
        break;
    case JCONF_NODE_TYPE_BOOL:
        node->d_integer = va_arg(ap, jint) != 0;
        break;
    case JCONF_NODE_TYPE_FLOAT:
        node->d_floating = va_arg(ap, jdouble);
        break;
    case JCONF_NODE_TYPE_OBJECT:
        node->d_object = va_arg(ap, JConfNode *);
        break;
    case JCONF_NODE_TYPE_ARRAY:
        node->d_array = va_arg(ap, JPtrArray *);
        break;
    default:
        node->type = JCONF_NODE_TYPE_NULL;
        break;
    }
    return node;
}

/*
 * 初始化结点数组
 */
static inline JPtrArray *j_conf_node_ptr_array_new()
{
    JPtrArray *array =
        j_ptr_array_new_full(0, (JDestroyNotify) j_object_unref);
    return array;
}

static void j_conf_node_free(JConfNode * node)
{
    j_free(node->name);
    switch (node->type) {
    case JCONF_NODE_TYPE_STRING:
        j_free(node->d_string);
        break;
    case JCONF_NODE_TYPE_OBJECT:
        j_conf_node_unref(node->d_object);
        break;
    case JCONF_NODE_TYPE_ARRAY:
        j_ptr_array_free(node->d_array, TRUE);
        break;
    default:
        break;
    }
}
