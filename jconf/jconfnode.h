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
#ifndef __JCONF_NODE_H__
#define __JCONF_NODE_H__

#include <jlib/jlib.h>

/**
 * JConfNode:
 * 
 * The `JConfNode` struct is opaque data type
 * presenting a kind of data
 */
typedef struct _JConfNode JConfNode;

typedef enum {
    JCONF_NODE_TYPE_NULL = 0,   /* 空值 */
    JCONF_NODE_TYPE_STRING,     /* 字符串 */
    JCONF_NODE_TYPE_INTEGER,    /* 整数 */
    JCONF_NODE_TYPE_FLOAT,      /* 浮点数 */
    JCONF_NODE_TYPE_BOOL,       /* 逻辑值 */
    JCONF_NODE_TYPE_OBJECT,     /* 对象 */
    JCONF_NODE_TYPE_ARRAY       /* 数组 */
} JConfNodeType;


#define j_conf_node_ref(n)  J_OBJECT_REF(n)
#define j_conf_node_unref(n) J_OBJECT_UNREF(n)

JConfNodeType j_conf_node_get_type(JConfNode * node);
const jchar *j_conf_node_get_name(JConfNode * node);

typedef JConfNode JConfIntegerNode;
jboolean j_conf_node_is_integer(JConfNode * node);
jint64 j_conf_integer_node_get(JConfIntegerNode * node);

typedef JConfNode JConfArrayNode;
jboolean j_conf_node_is_array(JConfNode * node);
juint j_conf_array_node_get_length(JConfArrayNode * node);
JConfNode *j_conf_array_node_get(JConfArrayNode * node, juint index);

#endif
