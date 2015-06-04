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

#ifndef __J_LIB_LIST_H__
#define __J_LIB_LIST_H__

#include "jtypes.h"

typedef struct _JList JList;

struct _JList {
    jpointer data;
    JList *prev;
    JList *next;
};
#define j_list_next(l)  ((l)->next)
#define j_list_prev(l)  ((l)->prev)
#define j_list_data(l)  ((l)->data)

#define j_list_new()    (NULL)


JList *j_list_alloc(jpointer data);


/*
 * Gets the number of elements in a JSList
 * This function iterates over the whole list to count its elements.
 */
juint j_list_length(JList * l);


/*
 * Appends a new element with data to last of the list
 */
JList *j_list_append(JList * l, jpointer data);

/*
 * Prepends a new element with data to the head of the list.
 * It's faster than j_list_append()
 */
JList *j_list_prepend(JList * l, jpointer data);

/*
 * 查找包含data的元素
 */
JList *j_list_find(JList * l, jconstpointer data);
jpointer j_list_find_custom(JList * l, JCompareFunc compare,
                            jconstpointer user_data);

/*
 * Returns the first element of the list
 */
JList *j_list_first(JList * l);

/*
 * Returns the last element of the list
 */
JList *j_list_last(JList * l);

/*
 * Frees the list
 */
void j_list_free(JList * l);
/*
 * Frees the list and all data using JListDestroy
 */
void j_list_free_full(JList * l, JDestroyNotify destroy);

void j_list_free1(JList * l, JDestroyNotify destroy);


/*
 * Compares two list
 */
int j_list_compare(JList * l1, JList * l2, JCompareFunc compare);

/*
 * Removes an element from a JList. 
 * If two or more elements  contain the same data, only the first one is removed.
 * If none of the elements contain the data, JList is unchanged.
 */
JList *j_list_remove(JList * l, jpointer data);
JList *j_list_remove_link(JList * l, JList * link);

/*
 * Removes the node link from the list and frees it
 */
JList *j_list_delete_link(JList * l, JList * link);

JList *j_list_insert_before(JList * list, JList * sibling, jpointer data);

/*
 * 将双向列表倒转
 */
JList *j_list_reverse(JList * list);


/*
 * 排序
 */
JList *j_list_sort_with_data(JList * list, JCompareDataFunc compare,
                             jpointer user_data);

/*
 * 查找link在list中的位置 
 */
jint j_list_position(JList * list, JList * link);
/*
 * 查找data在list中的位置
 */
jint j_list_index(JList * list, jconstpointer data);


#endif
