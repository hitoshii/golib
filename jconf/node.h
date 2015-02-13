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
#ifndef __J_CONF_NODE_H__
#define __J_CONF_NODE_H__

#include "type.h"
#include <jlib/jlist.h>

typedef struct _JConfNode JConfNode;
typedef struct _JConfNode JConfDirective;
typedef struct _JConfNode JConfScope;


const char *j_conf_node_get_name(JConfNode * n);
JList *j_conf_node_get_arguments(JConfNode * n);
JList *j_conf_node_get_children(JConfNode * n);


typedef enum {
    J_CONF_NODE_DIRECTIVE,
    J_CONF_NODE_SCOPE,
} JConfNodeType;

JConfNodeType j_conf_node_get_type(JConfNode * n);
#define J_CONF_NODE_TYPE(n)  j_conf_node_get_type(n)

#define j_conf_node_is_directive(n)  (J_CONF_NODE_TYPE(n)==J_CONF_NODE_DIRECTIVE)
#define j_conf_node_is_scope(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_SCOPE)


JConfNode *j_conf_node_new(JConfNodeType type, const char *name);
JConfNode *j_conf_node_new_take(JConfNodeType type, char *name);
void j_conf_node_free(JConfNode * n);

int j_conf_node_set_arguments(JConfNode * n, const char *raw);
int j_conf_node_set_arguments_take(JConfNode * n, char *raw);

void j_conf_node_append_child(JConfNode * n, JConfNode * child);


/*
 * Gets children scopes
 */
JList *j_conf_node_get_scope(JConfNode *n,const char *name);

/*
 * Joins two nodes
 * The type of two nodes must be the same, it not this function simply returns.
 * For J_CONF_NODE_DIRECTIVE,
 *              if names of two nodes are the same, n2 will override n1
 *              if names of two ndoes are different, nothing is done
 * For J_CONF_NODE_SCOPE,
 *              if names and arguments of two nodes are the same,
 *                      the children of n2 will be joined into the children of n1
 *              if ...
 */
int j_conf_node_join(JConfNode * n1, JConfNode * n2);


#endif
