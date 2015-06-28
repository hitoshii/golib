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
#ifndef __JLIB_SLIST_H__
#define __JLIB_SLIST_H__

#include "jtypes.h"

/*
 * Singly linked list
 */

typedef struct _JSList JSList;


struct _JSList {
    jpointer data;
    JSList *next;
};

#define j_slist_new()   (NULL)
#define j_slist_next(l) ((l)->next)
#define j_slist_data(l) ((l)->data)

JSList *j_slist_alloc(jpointer data);

JSList *j_slist_last(JSList * l);
JSList *j_slist_append(JSList * l, jpointer data);
JSList *j_slist_preppend(JSList * l, jpointer data);

JSList *j_slist_remove(JSList * l, jconstpointer data);

void j_slist_free(JSList * l);
void j_slist_free_full(JSList * l, JDestroyNotify destroy);

void j_slist_free1(JSList * l, JDestroyNotify destroy);

JSList *j_slist_find(JSList * l, jpointer data);
JSList *j_slist_find_custom(JSList * l, JCompareFunc compare,
                            jconstpointer user_data);
jpointer j_slist_find_data_custom(JSList * l, JCompareFunc compare,
                                  jconstpointer user_data);


/*
 * Gets the number of elements in a JSList
 * This function iterates over the whole list to count its elements.
 */
juint j_slist_length(JSList * l);

/*
 * Removes the node link_ from the list and frees it.
 * Compare this to g_slist_remove_link() which removes the node without freeing it.
 */
JSList *j_slist_delete_link(JSList * l, JSList * e);
JSList *j_slist_remove_link(JSList * l, JSList * e);


#endif
