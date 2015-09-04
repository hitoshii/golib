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
#ifndef __JLIB_SLIST_H__
#define __JLIB_SLIST_H__

#include "jtypes.h"

/*
 * Singly linked list
 */

typedef struct _JSList JSList;


struct _JSList {
    void * data;
    JSList *next;
};

#define j_slist_new()   (NULL)
#define j_slist_next(l) ((l)->next)
#define j_slist_data(l) ((l)->data)

JSList *j_slist_alloc(void * data);

JSList *j_slist_last(JSList * l);
JSList *j_slist_append(JSList * l, void * data);
JSList *j_slist_preppend(JSList * l, void * data);

JSList *j_slist_remove(JSList * l, const void * data);

void j_slist_free(JSList * l);
void j_slist_free_full(JSList * l, JDestroyNotify destroy);

void j_slist_free1(JSList * l, JDestroyNotify destroy);

JSList *j_slist_find(JSList * l, void * data);
JSList *j_slist_find_custom(JSList * l, JCompareFunc compare,
                            const void * user_data);
void * j_slist_find_data_custom(JSList * l, JCompareFunc compare,
                                const void * user_data);


/*
 * Gets the number of elements in a JSList
 * This function iterates over the whole list to count its elements.
 */
unsigned int j_slist_length(JSList * l);

/*
 * Removes the node link_ from the list and frees it.
 * Compare this to g_slist_remove_link() which removes the node without freeing it.
 */
JSList *j_slist_delete_link(JSList * l, JSList * e);
JSList *j_slist_remove_link(JSList * l, JSList * e);


#endif
