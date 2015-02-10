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
#include "node.h"
#include <jlib.h>
#include <stdint.h>


struct _JConfNode {
    JConfNodeType type;
    char *name;
    JList *args;
    JList *children;
};

const char *j_conf_node_get_name(JConfNode * n)
{
    return n->name;
}

JList *j_conf_node_get_arguments(JConfNode * n)
{
    return n->args;
}

JList *j_conf_node_get_children(JConfNode * n)
{
    return n->children;
}

JConfNodeType j_conf_node_get_type(JConfNode * n)
{
    return n->type;
}

JConfNode *j_conf_node_new(JConfNodeType type, const char *name)
{
    JConfNode *n = (JConfNode *) j_malloc(sizeof(JConfNode));
    n->name = j_strdup(name);
    n->type = type;
    n->args = NULL;
    n->children = NULL;
    return n;
}

void j_conf_node_free(JConfNode * n)
{
    j_free(n->name);
    j_list_free_full(n->args, (JListDestroy) j_conf_data_free);
    if (j_conf_node_is_scope(n)) {
        j_list_free_full(n->children, (JListDestroy) j_conf_node_free);
    }
    j_free(n);
}


static inline void j_conf_node_append_argument(JConfNode * n,
                                               const char *raw)
{
    n->args = j_list_append(n->args, j_conf_data_parse(raw));
}

typedef enum {
    J_ARGUMENT_NEW,
    J_ARGUMENT_RAW,
    J_ARGUMENT_STRING,
    J_ARGUMENT_STRING_ESCAPE,
} JConfNodeArgumentState;

int j_conf_node_set_arguments(JConfNode * n, const char *raw)
{
    if (raw == NULL) {
        return 1;
    }
    while (*raw) {
        if (*raw != ' ') {
            JConfNodeArgumentState state = J_ARGUMENT_NEW;
            const char *start = raw;
            const char *ptr = start;
            while (*ptr) {
                char c = *ptr;
                if (state == J_ARGUMENT_NEW) {
                    if (c == '\"') {
                        state = J_ARGUMENT_STRING;
                    } else {
                        state = J_ARGUMENT_RAW;
                    }
                } else if (state == J_ARGUMENT_STRING) {
                    if (c == '\\') {
                        state = J_ARGUMENT_STRING_ESCAPE;
                    } else if (c == '\"') {
                        char *str = j_strndup(start, ptr - start + 1);
                        j_conf_node_append_argument(n, str);
                        j_free(str);
                        break;
                    }
                } else if (state == J_ARGUMENT_STRING_ESCAPE) {
                    state = J_ARGUMENT_STRING;
                } else if (state == J_ARGUMENT_RAW) {
                    if (c == ' ') {
                        char *str = j_strndup(start, ptr - start + 1);
                        j_conf_node_append_argument(n, str);
                        j_free(str);
                        break;
                    }
                }
                ptr++;
            }
            if (*ptr == '\0') {
                if (state == J_ARGUMENT_STRING_ESCAPE) {
                    return 0;
                } else if (state == J_ARGUMENT_RAW) {
                    char *str = j_strndup(start, ptr - start + 1);
                    j_conf_node_append_argument(n, str);
                    j_free(str);
                }
            }
            raw = ptr;
        }
        raw++;
    }
    return 1;
}

void j_conf_node_append_child(JConfNode * n, JConfNode * child)
{
    n->children = j_list_append(n->children, child);
}


int j_conf_node_join(JConfNode * n1, JConfNode * n2)
{
    if (j_conf_node_get_type(n1) != j_conf_node_get_type(n2)) {
        return 0;
    }
    const char *name1 = j_conf_node_get_name(n1);
    const char *name2 = j_conf_node_get_name(n2);
    if (j_strcmp0(name1, name2)) {
        return 0;
    }
    if (j_conf_node_is_directive(n1)) {
        j_list_free_full(n1->args, (JListDestroy) j_conf_data_free);
        n1->args = n2->args;
        n2->args = NULL;
    } else {
        JList *arg1 = j_conf_node_get_arguments(n1);
        JList *arg2 = j_conf_node_get_arguments(n2);
        if (j_list_compare(arg1, arg2, (JListCompare) j_conf_data_compare)) {
            return 0;
        }
        /* joins two scope */
        JList *nodes2 = j_conf_node_get_children(n2);
        while (nodes2) {
            JConfNode *c2 = (JConfNode *) j_list_data(nodes2);
            JList *nodes1 = j_conf_node_get_children(n1);
            while (nodes1) {
                JConfNode *c1 = (JConfNode *) j_list_data(nodes1);
                if (j_conf_node_join(c1, c2)) {
                    c2 = NULL;
                    break;
                }
                nodes1 = j_list_next(nodes1);
            }
            if (c2) {
                j_conf_node_append_child(n1, c2);
            }
            nodes2 = j_list_next(nodes2);
        }
        j_list_free(n2->children);
        n2->children = NULL;
    }
    j_conf_node_free(n2);
    return 1;
}
