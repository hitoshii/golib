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
#include "jslist.h"
#include "jmem.h"


JSList *j_slist_alloc(jpointer data)
{
    JSList *l = (JSList *) j_malloc(sizeof(JSList));
    l->data = data;
    l->next = NULL;
    return l;
}

JSList *j_slist_last(JSList * l)
{
    if (l == NULL) {
        return NULL;
    }
    while (j_slist_next(l)) {
        l = j_slist_next(l);
    }
    return l;
}

JSList *j_slist_append(JSList * l, jpointer data)
{
    JSList *last = j_slist_last(l);
    if (last == NULL) {
        return j_slist_alloc(data);
    }
    last->next = j_slist_alloc(data);
    return l;
}

JSList *j_slist_preppend(JSList * l, jpointer data)
{
    JSList *h = j_slist_alloc(data);
    h->next = l;
    return h;
}
