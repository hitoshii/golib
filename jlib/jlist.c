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


#include "jlist.h"
#include "jmem.h"
#include <stdlib.h>


JList *j_list_append(JList * l, void *data)
{
    JList *new = j_list_alloc(data);
    if (l == NULL) {
        return new;
    }
    JList *last = j_list_last(l);
    last->next = new;
    new->prev = last;
    return l;
}

JList *j_list_prepend(JList * l, void *data)
{
    JList *new = j_list_alloc(data);
    if (l == NULL) {
        return new;
    }
    JList *first = j_list_first(l);
    first->prev = new;
    new->next = first;
    return new;
}

JList *j_list_first(JList * l)
{
    if (l == NULL) {
        return NULL;
    }
    while (j_list_prev(l)) {
        l = j_list_prev(l);
    }
    return l;
}

JList *j_list_last(JList * l)
{
    if (l == NULL) {
        return NULL;
    }
    while (j_list_next(l)) {
        l = j_list_next(l);
    }
    return l;
}

JList *j_list_find(JList * l, JListCompare compare, const void *user_data)
{
    JList *ptr = l;
    while (ptr) {
        const void *data = j_list_data(ptr);
        if (compare(data, user_data) == 0) {
            return ptr;
        }
        ptr = j_list_next(ptr);
    }
    return NULL;
}

void *j_list_find_data(JList * l, JListCompare compare,
                       const void *user_data)
{
    JList *ptr = j_list_find(l, compare, user_data);
    if (ptr) {
        return j_list_data(ptr);
    }
    return NULL;
}


JList *j_list_alloc(void *data)
{
    JList *l = (JList *) j_malloc(sizeof(JList));
    l->data = data;
    l->prev = NULL;
    l->next = NULL;
    return l;
}

void j_list_free1(JList * l, JListDestroy destroy)
{
    if (destroy) {
        destroy(j_list_data(l));
    }
    j_free(l);
}

void j_list_free(JList * l)
{
    j_list_free_full(l, NULL);
}

/*
 * Frees the list and all data using JListDestroy
 */
void j_list_free_full(JList * l, JListDestroy destroy)
{
    if (l == NULL) {
        return;
    }
    do {
        JList *next = j_list_next(l);
        j_list_free1(l, destroy);
        l = next;
    } while (l);
}

int j_list_compare(JList * l1, JList * l2, JListCompare compare)
{
    while (l1 && l2) {
        int ret = compare(j_list_data(l1), j_list_data(l2));
        if (ret) {
            return ret;
        }
        l1 = j_list_next(l1);
        l2 = j_list_next(l2);
    }
    return l1 == l2;
}
