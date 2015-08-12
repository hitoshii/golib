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
#ifndef __JLIB_ARRAY_H__
#define __JLIB_ARRAY_H__

#include "jtypes.h"

typedef struct {
    juint8 *data;
    juint len;
} JByteArray;
#define j_byte_array_get_data(ba)   ((ba)->data)
#define j_byte_array_get_len(ba)    ((ba)->len)


JByteArray *j_byte_array_new(void);
JByteArray *j_byte_array_sized_new(juint size);

void j_byte_array_append(JByteArray * ba, const juint8 * data, juint len);
void j_byte_array_preppend(JByteArray * ba, const juint8 * data,
                           juint len);

juint8 *j_byte_array_free(JByteArray * ba, jboolean f);

void j_byte_array_clear(JByteArray * ba);


typedef struct {
    jpointer *data;
    juint len;
} JPtrArray;
#define j_ptr_array_get_data(pa)    ((pa)->data)
#define j_ptr_array_get_len(pa)     ((pa)->len)

JPtrArray *j_ptr_array_new(void);
JPtrArray *j_ptr_array_new_full(juint size, JDestroyNotify destroy);

void j_ptr_array_set_size(JPtrArray * pa, juint size);
void j_ptr_array_remove_range(JPtrArray * pa, juint index, juint length);

void j_ptr_array_set_free(JPtrArray * pa, JDestroyNotify destroy);

/*
 * Returns the position of the new-added pointer
 */
juint j_ptr_array_append_ptr(JPtrArray * pa, jpointer ptr);
void j_ptr_array_append(JPtrArray * pa, ...);   /* Ends with NULL */
void j_ptr_array_insert(JPtrArray * pa, jpointer ptr, juint index);

#define j_ptr_array_get_ptr(a, i)  ((a)->data[i])
jpointer j_ptr_array_get(JPtrArray * pa, juint index);
jpointer j_ptr_array_find(JPtrArray * pa, JCompareFunc compare,
                          jpointer user_data);
jint j_ptr_array_find_index(JPtrArray * pa, JCompareFunc compare,
                            jpointer user_data);

/* 判断数组中是否包含元素data */
jboolean j_ptr_array_contains(JPtrArray * array, jpointer data);
/* 插入指针，保证不重复 */
void j_ptr_array_append_ptr_unique(JPtrArray * array, jpointer data);

/*
 * Removes the first occurrence of the given pointer from the pointer array.
 * The following elements are moved down one place.
 * If array has a non-NULL GDestroyNotify function it is called for the removed element.
 */
jboolean j_ptr_array_remove(JPtrArray * pa, jpointer ptr);

/*
 * Removes the pointer at the given index from the pointer array.
 * The following elements are moved down one place.
 * If array has a non-NULL GDestroyNotify function it is called for the removed element.
 */
jboolean j_ptr_array_remove_index(JPtrArray * pa, juint index);

jboolean j_ptr_array_remove_index_fast(JPtrArray * pa, juint index);


void j_ptr_array_free(JPtrArray * pa, jboolean free_ptr);

#endif
