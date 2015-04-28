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

JSList *j_slist_alloc(jpointer data);

JSList *j_slist_last(JSList * l);
JSList *j_slist_append(JSList * l, jpointer data);
JSList *j_slist_preppend(JSList * l, jpointer data);


#endif
