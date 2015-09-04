/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
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

boolean j_conf_node_is_null(JConfNode * node);
boolean j_conf_node_is_integer(JConfNode * node);
boolean j_conf_node_is_string(JConfNode * node);
boolean j_conf_node_is_float(JConfNode * node);
boolean j_conf_node_is_bool(JConfNode * node);
boolean j_conf_node_is_object(JConfNode * node);
boolean j_conf_node_is_array(JConfNode * array);

int64_t j_conf_integer_get(JConfInteger * node);
void j_conf_integer_set(JConfInteger * node, int64_t integer);
const char *j_conf_string_get(JConfString * node);
double j_conf_float_get(JConfFloat * node);
boolean j_conf_bool_get(JConfBool * node);

unsigned int j_conf_array_get_length(JConfArray * node);
JConfNode *j_conf_array_get(JConfArray * node, unsigned int index);
void j_conf_array_append(JConfArray * array, JConfNode * node);
void j_conf_array_append_integer(JConfArray * array, int64_t integer);
void j_conf_array_append_string(JConfArray * array, const char * string);
void j_conf_array_append_bool(JConfArray * array, boolean b);
void j_conf_array_append_float(JConfArray * array, double floating);

JConfNode *j_conf_object_get(JConfObject * node, const char * name);
void j_conf_object_set(JConfObject * node, const char * name,
                       JConfNode * child);
void j_conf_object_set_take(JConfObject * node, char * name,
                            JConfNode * child);
void j_conf_object_set_integer(JConfObject * node, const char * name,
                               int64_t integer);
void j_conf_object_set_string(JConfObject * node, const char * name,
                              const char * string);
void j_conf_object_set_bool(JConfObject * node, const char * name,
                            boolean b);
void j_conf_object_set_float(JConfObject * node, const char * name,
                             double floating);
void j_conf_object_set_null(JConfObject * node, const char * name);
/*
 * j_conf_object_get_keys:
 * @node: JConfObject
 *
 * Gets all keys
 * Returns: 不要修改JPtrArray
 */
JPtrArray *j_conf_object_get_keys(JConfObject * node);
void j_conf_object_remove(JConfObject * node, const char * name);

/*
 * j_conf_object_get_integer:
 * j_conf_object_get_string
 * j_conf_object_get_bool
 * @node: JConfObject
 * @name: 键名
 * @def: 如果不存在，使用该默认值
 *
 * Returns:
 */
int64_t j_conf_object_get_integer(JConfObject *node, const char *name, int64_t def);
const char *j_conf_object_get_string(JConfObject *node, const char *name, const char *def);
boolean j_conf_object_get_bool(JConfObject *node, const char *name, boolean def);
/*
 */
JList *j_conf_object_get_string_list(JConfObject *node, const char *name);

/*
 * j_conf_object_get_integer_priority
 * j_conf_object_get_string_priority
 * j_conf_object_get_bool_priority
 *
 * @root: 根节点
 * @node: 优先的节点
 * @name: 键名
 * @def: 默认值
 *
 * 先从node中寻找name，如果找到，返回该值
 * 再从root中寻找name，如果找到，返回该值
 * 返回默认值def
 */
int64_t j_conf_object_get_integer_priority(JConfObject *root, JConfObject *node,
        const char *name, int64_t def);
const char *j_conf_object_get_string_priority(JConfObject *root, JConfObject *node,
        const char *name, const char *def);
boolean j_conf_object_get_bool_priority(JConfObject *root, JConfObject *node,
                                        const char *name, boolean def);

JList *j_conf_object_get_string_list_priority(JConfObject *root, JConfObject *node,
        const char *name);
/*
 * j_conf_object_lookup:
 * @node: JConfObject
 * @rexp: 正则表达式
 * @type: 节点的类型
 *
 * Returns: 返回一个包含键值的列表，使用j_list_free()释放
 */
JList *j_conf_object_lookup(JConfObject *node, const char *rexp, JConfNodeType type);

/* 转化为字符串格式 */
char *j_conf_node_dump(JConfNode *node);

#endif
