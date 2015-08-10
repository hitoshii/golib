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
#ifndef __J_CONF_NODE_H__
#define __J_CONF_NODE_H__

#include <jlib/jlib.h>

/**
 * JConfNode:
 *
 * The `JConfNode` struct is opaque data type
 * presenting a kind of data
 */
typedef struct _JConfNode JConfNode;
typedef struct _JConfNode JConfObject;
typedef struct _JConfNode JConfNull;
typedef struct _JConfNode JConfInteger;
typedef struct _JConfNode JConfString;
typedef struct _JConfNode JConfArray;
typedef struct _JConfNode JConfBool;
typedef struct _JConfNode JConfFloat;

typedef enum {
    J_CONF_NODE_TYPE_NULL = 0,  /* 空值 */
    J_CONF_NODE_TYPE_STRING,    /* 字符串 */
    J_CONF_NODE_TYPE_INTEGER,   /* 整数 */
    J_CONF_NODE_TYPE_FLOAT,     /* 浮点数 */
    J_CONF_NODE_TYPE_BOOL,      /* 逻辑值 */
    J_CONF_NODE_TYPE_OBJECT,    /* 对象 */
    J_CONF_NODE_TYPE_ARRAY      /* 数组 */
} JConfNodeType;

#define j_conf_node_ref(n)  J_OBJECT_REF(n)
#define j_conf_node_unref(n) J_OBJECT_UNREF(n)

JConfNode *j_conf_node_new(JConfNodeType type, ...);

JConfNodeType j_conf_node_get_type(JConfNode * node);

jboolean j_conf_node_is_null(JConfNode * node);
jboolean j_conf_node_is_integer(JConfNode * node);
jboolean j_conf_node_is_string(JConfNode * node);
jboolean j_conf_node_is_float(JConfNode * node);
jboolean j_conf_node_is_bool(JConfNode * node);
jboolean j_conf_node_is_object(JConfNode * node);
jboolean j_conf_node_is_array(JConfNode * array);

jint64 j_conf_integer_get(JConfInteger * node);
void j_conf_integer_set(JConfInteger * node, jint64 integer);
const jchar *j_conf_string_get(JConfString * node);
jdouble j_conf_float_get(JConfFloat * node);
jboolean j_conf_bool_get(JConfBool * node);

juint j_conf_array_get_length(JConfArray * node);
JConfNode *j_conf_array_get(JConfArray * node, juint index);
void j_conf_array_append(JConfArray * array, JConfNode * node);
void j_conf_array_append_integer(JConfArray * array, jint64 integer);
void j_conf_array_append_string(JConfArray * array, const jchar * string);
void j_conf_array_append_bool(JConfArray * array, jboolean b);
void j_conf_array_append_float(JConfArray * array, jdouble floating);

JConfNode *j_conf_object_get(JConfObject * node, const jchar * name);
void j_conf_object_set(JConfObject * node, const jchar * name,
                       JConfNode * child);
void j_conf_object_set_take(JConfObject * node, jchar * name,
                            JConfNode * child);
void j_conf_object_set_integer(JConfObject * node, const jchar * name,
                               jint64 integer);
void j_conf_object_set_string(JConfObject * node, const jchar * name,
                              const jchar * string);
void j_conf_object_set_bool(JConfObject * node, const jchar * name,
                            jboolean b);
void j_conf_object_set_float(JConfObject * node, const jchar * name,
                             jdouble floating);
void j_conf_object_set_null(JConfObject * node, const jchar * name);
JPtrArray *j_conf_object_get_keys(JConfObject * node);
void j_conf_object_remove(JConfObject * node, const jchar * name);

#endif
