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
#ifndef __JCONF_STRUCT_H__
#define __JCONF_STRUCT_H__

#include <jlib/jlib.h>
#include <inttypes.h>

typedef struct _JConfNode JConfNode;

typedef enum {
    J_CONF_NODE_OBJECT,
    J_CONF_NODE_ARRAY,
    J_CONF_NODE_INT,
    J_CONF_NODE_FLOAT,
    J_CONF_NODE_STRING,
    J_CONF_NODE_TRUE,
    J_CONF_NODE_FALSE,
    J_CONF_NODE_NULL,
} JConfNodeType;

struct _JConfNode {
    JConfNodeType type;
    union {
        JList *children;
        char *string;
        int64_t integer;
        double floating;
    } data;
    char *name;
};
#define j_conf_node_get_type(n) ((n)->type)
#define j_conf_node_is_object(n) (j_conf_node_get_type(n)==J_CONF_NODE_OBJECT)
#define j_conf_node_is_array(n) (j_conf_node_get_type(n)==J_CONF_NODE_ARRAY)
#define j_conf_node_is_int(n)   (j_conf_node_get_type(n)==J_CONF_NODE_INT)
#define j_conf_node_is_float(n) (j_conf_node_get_type(n)==J_CONF_NODE_FLOAT)
#define j_conf_node_is_string(n) (j_conf_node_get_type(n)==J_CONF_NODE_STRING)
#define j_conf_node_is_true(n)  (j_conf_node_get_type(n)==J_CONF_NODE_TRUE)
#define j_conf_node_is_false(n) (j_conf_node_get_type(n)==J_CONF_NODE_FALSE)
#define j_conf_node_is_null(n)  (j_conf_node_get_type(n)==J_CONF_NODE_NULL)
#define j_conf_node_is_number(n)    (j_conf_node_is_int(n)||j_conf_node_is_float(n))
#define j_conf_node_is_bool(n)  (j_conf_node_is_true(n)||j_conf_node_is_false(n))
#define j_conf_node_get_name(n) ((n)->name)

void j_conf_node_append_child(JConfNode * node, JConfNode * child);
JList *j_conf_node_get_children(JConfNode * node);


JConfNode *j_conf_node_create_object(void);
JConfNode *j_conf_node_create_array(void);
JConfNode *j_conf_node_create_int(int64_t integer);
JConfNode *j_conf_node_create_float(double floatig);
JConfNode *j_conf_node_create_string(const char *string);
JConfNode *j_conf_node_create_string_take(char *string);
JConfNode *j_conf_node_create_true(void);
JConfNode *j_conf_node_create_false(void);
JConfNode *j_conf_node_create_null(void);

/*
 * 获取最后一个名为name的节点
 */
JConfNode *j_conf_object_get(JConfNode * node, const char *name);
int64_t j_conf_object_get_int(JConfNode * node, const char *name,
                              int64_t def);
const char *j_conf_object_get_string(JConfNode * node, const char *name,
                                     const char *def);
double j_conf_object_get_float(JConfNode * node, const char *name,
                               double def);
double j_conf_object_get_number(JConfNode * node, const char *name,
                                double def);
int j_conf_object_get_bool(JConfNode * node, const char *name, int def);
/*
 * 获取所有名为name的子节点
 */
JList *j_conf_object_get_list(JConfNode * node, const char *name);
/*
 * 获取值
 */
int64_t j_conf_int_get(JConfNode * node);
double j_conf_float_get(JConfNode * node);
const char *j_conf_string_get(JConfNode * node);
int j_conf_bool_get(JConfNode * node);



void j_conf_node_set_name(JConfNode * node, const char *name);
void j_conf_node_set_name_take(JConfNode * node, char *name);

void j_conf_node_free(JConfNode * node);


#define J_CONF_SUCCESS 0
#define J_CONF_ERROR_FILE   1   /* 打开文件错误 */
#define J_CONF_ERROR_MALFORMED 2    /* 格式错误 */
typedef struct {
    JList *children;            /* JConfNode */
    char *name;                 /* file name */
    unsigned int line;          /* line number */

    /* private */
    JList *vars;                /* 变量 */
    JStack *stack;              /* 用来保存当前解析位置,文件名:行号 */
} JConfRoot;
#define j_conf_root_get_children(r) ((r)->children)
#define j_conf_root_get_name(r)     ((r)->name)
#define j_conf_root_line_add(r)     (r)->line++
#define j_conf_root_get_line(r)     ((r)->line)
JConfRoot *j_conf_root_new(const char *name);
void j_conf_root_free(JConfRoot * root);
void j_conf_root_append(JConfRoot * root, JConfNode * node);

void j_conf_root_push(JConfRoot * root, const char *path);
void j_conf_root_pop(JConfRoot * root);

/*
 * 添加变量
 */
void j_conf_root_add_var(JConfRoot * root, const char *name,
                         const char *value);
/*
 * 添加多个变量，变量名与变量值对
 * 以NULL结尾
 */
void j_conf_root_add_vars(JConfRoot * root, ...);
char *j_conf_root_assign(JConfRoot * root, char *string);   /* 替换字符串中的变量 */

/*
 * 获取子节点
 */
JConfNode *j_conf_root_get(JConfRoot * root, const char *name);
JList *J_conf_root_get_list(JConfRoot * root, const char *name);

/*
 * 获取子节点值
 */
int64_t j_conf_root_get_int(JConfRoot * root, const char *name,
                            int64_t def);
const char *j_conf_root_get_string(JConfRoot * root, const char *name,
                                   const char *def);
double j_conf_root_get_float(JConfRoot * root, const char *name,
                             double def);
double j_conf_root_get_number(JConfRoot * root, const char *name,
                              double def);
int j_conf_root_get_bool(JConfRoot * root, const char *name, int def);


#endif
