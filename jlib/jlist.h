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
 * Finds an element in a JList, using a supplied function to find the desired element. 
 * It iterates over the list, calling the given function which should return 0 
 * when the desired element is found. 
 * The function takes two gconstpointer arguments, the JList element's data 
 * as the first argument and the given user data.
 */
JList *j_list_find(JList * l, JCompareFunc compare,
                   jconstpointer user_data);
jpointer j_list_find_data(JList * l, JCompareFunc compare,
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


#endif
