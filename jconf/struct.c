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
#include <stdarg.h>
#include <stdio.h>

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

int64_t j_conf_object_get_int(JConfNode * obj, const char *name,
                              int64_t def)
{
    JConfNode *node = j_conf_object_get(obj, name);
    if (node == NULL || !j_conf_node_is_int(node)) {
        return def;
    }
    return j_conf_int_get(node);
}

const char *j_conf_object_get_string(JConfNode * obj, const char *name,
                                     const char *def)
{
    JConfNode *node = j_conf_object_get(obj, name);
    if (node == NULL || !j_conf_node_is_string(node)) {
        return def;
    }
    return j_conf_string_get(node);
}

double j_conf_object_get_float(JConfNode * obj, const char *name,
                               double def)
{
    JConfNode *node = j_conf_object_get(obj, name);
    if (node == NULL || !j_conf_node_is_float(node)) {
        return def;
    }
    return j_conf_float_get(node);
}

double j_conf_object_get_number(JConfNode * obj, const char *name,
                                double def)
{
    JConfNode *node = j_conf_object_get(obj, name);
    if (node == NULL || !j_conf_node_is_number(node)) {
        return def;
    }
    if (j_conf_node_is_int(node)) {
        return (double) j_conf_int_get(node);
    }
    return j_conf_float_get(node);
}

int j_conf_object_get_bool(JConfNode * obj, const char *name, int def)
{
    JConfNode *node = j_conf_object_get(obj, name);
    if (node == NULL || !j_conf_node_is_bool(node)) {
        return def;
    }
    return j_conf_bool_get(node);
}

/*
 * 获取所有名为name的子节点
 */
JList *j_conf_object_get_list(JConfNode * node, const char *name)
{
    JList *ret = NULL;
    JList *children = j_conf_node_get_children(node);
    while (children) {
        JConfNode *node = (JConfNode *) j_list_data(children);
        if (strcmp(name, j_conf_node_get_name(node)) == 0) {
            ret = j_list_append(ret, node);
        }
        children = j_list_next(children);
    }
    return ret;
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

int j_conf_bool_get(JConfNode * node)
{
    if (j_conf_node_is_true(node)) {
        return 1;
    }
    return 0;
}

/**********************************stack *******************************/

typedef struct {
    char *name;
    unsigned int line;
} JConfRootStack;

JConfRootStack *j_conf_root_stack_new(const char *path, unsigned int line)
{
    JConfRootStack *stack =
        (JConfRootStack *) j_malloc(sizeof(JConfRootStack));
    stack->name = j_strdup(path);
    stack->line = 0;
    return stack;
}

void j_conf_root_stack_free(JConfRootStack * stack)
{
    j_free(stack->name);
    j_free(stack);
}

/************************ variables ***************************************/
typedef struct {
    char *name;
    char *value;
} JConfVariable;

static inline JConfVariable *j_conf_variable_new(const char *name,
                                                 const char *value)
{
    JConfVariable *var = (JConfVariable *) j_malloc(sizeof(JConfVariable));
    var->name = j_strdup(name);
    var->value = j_strdup(value);
    return var;
}

static inline void j_conf_variable_free(JConfVariable * var)
{
    j_free(var->name);
    j_free(var->value);
    j_free(var);
}

/*************************************************************************/

JConfRoot *j_conf_root_new(const char *name)
{
    JConfRoot *root = (JConfRoot *) j_malloc(sizeof(JConfRoot));
    root->name = j_strdup(name);
    root->children = NULL;
    root->line = 1;
    root->stack = j_stack_new();
    root->vars = NULL;
    return root;
}

void j_conf_root_push(JConfRoot * root, const char *path)
{
    JConfRootStack *stack = j_conf_root_stack_new(path, root->line);
    j_stack_push(root->stack, stack);
    j_free(root->name);
    root->name = j_strdup(path);
    root->line = 1;
}

void j_conf_root_pop(JConfRoot * root)
{
    JConfRootStack *pop = j_stack_pop(root->stack);
    if (pop) {
        j_free(root->name);
        root->name = j_strdup(pop->name);
        root->line = pop->line;
        j_conf_root_stack_free(pop);
    }
}


void j_conf_root_free(JConfRoot * root)
{
    j_free(root->name);
    j_list_free_full(root->children, (JListDestroy) j_conf_node_free);
    j_list_free_full(root->vars, (JListDestroy) j_conf_variable_free);
    j_stack_free(root->stack, (JStackDestroy) j_conf_root_stack_free);
    j_free(root);
}

void j_conf_root_append(JConfRoot * root, JConfNode * node)
{
    root->children = j_list_append(root->children, node);
}


/*
 * 获取子节点
 */
JConfNode *j_conf_root_get(JConfRoot * root, const char *name)
{
    JConfNode *ret = NULL;
    JList *children = j_conf_root_get_children(root);
    while (children) {
        JConfNode *node = (JConfNode *) j_list_data(children);
        if (strcmp(name, j_conf_node_get_name(node)) == 0) {
            ret = node;
        }
        children = j_list_next(children);
    }
    return ret;
}

JList *j_conf_root_get_list(JConfRoot * root, const char *name)
{
    JList *ret = NULL;
    JList *children = j_conf_root_get_children(root);
    while (children) {
        JConfNode *node = (JConfNode *) j_list_data(children);
        if (strcmp(name, j_conf_node_get_name(node)) == 0) {
            ret = j_list_append(ret, node);
        }
        children = j_list_next(children);
    }
    return ret;
}


/*
 * 获取子节点值
 */
int64_t j_conf_root_get_int(JConfRoot * root, const char *name,
                            int64_t def)
{
    JConfNode *node = j_conf_root_get(root, name);
    if (node == NULL || !j_conf_node_is_int(node)) {
        return def;
    }
    return def;
}

const char *j_conf_root_get_string(JConfRoot * root, const char *name,
                                   const char *def)
{
    JConfNode *node = j_conf_root_get(root, name);
    if (node == NULL || !j_conf_node_is_string(node)) {
        return def;
    }
    return j_conf_string_get(node);
}

double j_conf_root_get_float(JConfRoot * root, const char *name,
                             double def)
{
    JConfNode *node = j_conf_root_get(root, name);
    if (node == NULL || !j_conf_node_is_float(node)) {
        return def;
    }
    return j_conf_float_get(node);
}

double j_conf_root_get_number(JConfRoot * root, const char *name,
                              double def)
{
    JConfNode *node = j_conf_root_get(root, name);
    if (node == NULL) {
        return def;
    }
    if (j_conf_node_is_float(node)) {
        return j_conf_float_get(node);
    } else if (j_conf_node_is_int(node)) {
        return (double) j_conf_int_get(node);
    }
    return def;
}

int j_conf_root_get_bool(JConfRoot * root, const char *name, int def)
{
    JConfNode *node = j_conf_root_get(root, name);
    if (node == NULL || !j_conf_node_is_bool(node)) {
        return def;
    }
    if (j_conf_node_is_true(node)) {
        return 1;
    }
    return 0;
}

/*
 * 添加变量
 */
void j_conf_root_add_var(JConfRoot * root, const char *name,
                         const char *value)
{
    JConfVariable *var = j_conf_variable_new(name, value);
    root->vars = j_list_append(root->vars, var);
}

/*
 * 添加多个变量，变量名与变量值对
 * 以NULL结尾
 */
void j_conf_root_add_vars(JConfRoot * root, ...)
{
    va_list ap;
    va_start(ap, root);
    const char *name = NULL;
    const char *value = NULL;
    while ((name = va_arg(ap, const char *))) {
        if (name == NULL) {
            break;
        }
        value = va_arg(ap, const char *);
        j_conf_root_add_var(root, name, value);
    }
    va_end(ap);
}

/*
 * 替换字符串中的变量
 */
char *j_conf_root_assign(JConfRoot * root, char *str)
{
    JList *vars = root->vars;
    char buf[1024];
    while (vars) {
        JConfVariable *var = (JConfVariable *) j_list_data(vars);
        snprintf(buf, sizeof(buf) / sizeof(char), "${%s}", var->name);
        str = j_str_replace(str, buf, var->value);
        vars = j_list_next(vars);
    }
    return str;
}
