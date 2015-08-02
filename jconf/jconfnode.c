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
        JHashTable *data_object;
        JPtrArray *data_array;
    } data;
};
#define d_string data.data_string
#define d_integer data.data_integer
#define d_floating data.data_floating
#define d_object data.data_object
#define d_array data.data_array

#define JCONF_NODE_TYPE(n)  ((n)->type)
#define JCONF_NODE_IS_ARRAY(n)  (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_ARRAY)
#define JCONF_NODE_IS_OBJECT(n) (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_OBJECT)
#define JCONF_NODE_IS_INTEGER(n) (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_INTEGER)
#define JCONF_NODE_IS_STRING(n) (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_STRING)
#define JCONF_NODE_IS_NULL(n) (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_NULL)
#define JCONF_NODE_IS_BOOL(n)   (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_BOOL)
#define JCONF_NODE_IS_FLOAT(n) (JCONF_NODE_TYPE(n)==JCONF_NODE_TYPE_FLOAT)


/**
 * j_conf_node_is_array:
 * @node JConfNode
 *
 * Checks to see if node presents a array value
 *
 * Returns TRUE if node has type of JCONF_NODE_TYPE_ARRAY, otherwise FALSE.
 */
jboolean j_conf_node_is_array(JConfNode * node)
{
    return JCONF_NODE_IS_ARRAY(node);
}

/**
 * j_conf_array_node_get_length:
 * @node: JConfArrayNode
 *
 * Returns: the length of array or zero
 */
juint j_conf_array_node_get_length(JConfArrayNode * node)
{
    if (J_UNLIKELY(!JCONF_NODE_IS_ARRAY(node))) {
        return 0;
    }
    return j_ptr_array_get_len(node->d_array);
}

/**
 * j_conf_array_node_get:
 * @node: JConfArrayNode
 * @index: the index of node to extract
 *
 * Returns: a JConfNode or %NULL if not found
 */
JConfNode *j_conf_array_node_get(JConfArrayNode * node, juint index)
{
    if (J_UNLIKELY(!JCONF_NODE_IS_ARRAY(node)
                   || index >= j_ptr_array_get_len(node->d_array))) {
        return NULL;
    }
    return j_ptr_array_get(node->d_array, index);
}

/**
 * j_conf_node_is_integer:
 * @node: JConfNode
 * 
 * Checks to see if node presents an integer value
 *
 * Returns: TRUE or FALSE
 */
jboolean j_conf_node_is_integer(JConfNode * node)
{
    return JCONF_NODE_IS_INTEGER(node);
}

/**
 * j_conf_integer_node_get:
 * @node: JConfIntegerNode
 *
 * Returns: the integer value of ndoe
 */
jint64 j_conf_integer_node_get(JConfIntegerNode * node)
{
    if (J_UNLIKELY(!JCONF_NODE_IS_INTEGER(node))) {
        return -1;
    }
    return node->d_integer;
}

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
static void j_conf_node_free(JConfNode * node);
static inline JPtrArray *j_conf_node_ptr_array_new(void);
static inline JHashTable *j_conf_node_object_table_new(void);

/*
 * 创建一个结点
 * 第三个参数根据type类型而定
 */
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
        node->d_object = va_arg(ap, JHashTable *);
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
 * 创建结点数组
 */
static inline JPtrArray *j_conf_node_ptr_array_new(void)
{
    JPtrArray *array =
        j_ptr_array_new_full(0, (JDestroyNotify) j_object_unref);
    return array;
}

/*
 * 创建对象存储表
 */
static inline JHashTable *j_conf_node_object_table_new(void)
{
    JHashTable *table = j_hash_table_new(5, j_str_hash, j_str_equal, NULL,
                                         (JValueDestroyFunc)
                                         j_object_unref);
    return table;
}


/*
 * 销毁一个结点
 */
static void j_conf_node_free(JConfNode * node)
{
    j_free(node->name);
    switch (node->type) {
    case JCONF_NODE_TYPE_STRING:
        j_free(node->d_string);
        break;
    case JCONF_NODE_TYPE_OBJECT:
        j_hash_table_free_full(node->d_object);
        break;
    case JCONF_NODE_TYPE_ARRAY:
        j_ptr_array_free(node->d_array, TRUE);
        break;
    default:
        break;
    }
}
