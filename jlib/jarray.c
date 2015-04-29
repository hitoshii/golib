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

#include "jarray.h"
#include "jmem.h"
#include <string.h>
#include <stdarg.h>


typedef struct {
    juint8 *data;
    juint len;
    juint total;                /* */
} JRealByteArray;

#define JByteArrayDefaultSize (1024)


static inline void j_byte_array_extend(JRealByteArray * ba)
{
    ba->total = ba->total << 1;
    ba->data = j_realloc(ba->data, ba->total);
}

static inline void j_byte_array_may_extend(JRealByteArray * real,
                                           juint len)
{
    while (real->total < len + real->len) {
        j_byte_array_extend(real);
    }
}


JByteArray *j_byte_array_new(void)
{
    return j_byte_array_sized_new(JByteArrayDefaultSize);
}

JByteArray *j_byte_array_sized_new(juint size)
{
    if (J_UNLIKELY(size == 0)) {
        size = JByteArrayDefaultSize;
    }
    JRealByteArray *real =
        (JRealByteArray *) j_malloc(sizeof(JRealByteArray));
    real->total = size;
    real->len = 0;
    real->data = j_malloc(sizeof(juint8) * real->total);
    return (JByteArray *) real;
}

void j_byte_array_append(JByteArray * ba, const juint8 * data, juint len)
{
    if (J_UNLIKELY(len == 0)) {
        return;
    }

    JRealByteArray *array = (JRealByteArray *) ba;
    j_byte_array_may_extend(array, len);

    memcpy(ba->data + ba->len, data, len);
    ba->len += len;
}

void j_byte_array_preppend(JByteArray * ba, const juint8 * data, juint len)
{
    if (J_UNLIKELY(len == 0)) {
        return;
    }
    JRealByteArray *real = (JRealByteArray *) ba;
    j_byte_array_may_extend(real, len);

    memmove(ba->data + len, ba->data, ba->len);
    memcpy(ba->data, data, len);
    ba->len += len;
}

void j_byte_array_clear(JByteArray * ba)
{
    ba->len = 0;
}

juint8 *j_byte_array_free(JByteArray * ba, jboolean f)
{
    juint8 *data = j_byte_array_get_data(ba);
    j_free(ba);
    if (f) {
        j_free(data);
        return NULL;
    }
    return data;
}



typedef struct {
    jpointer *data;
    juint len;
    juint total;
    JDestroyNotify free_func;
} JRealPtrArray;

#define JPtrArrayDefaultSize (32)

static inline void j_real_ptr_array_may_extend(JRealPtrArray * real,
                                               juint len)
{
    if (J_UNLIKELY(real->total < real->len + len)) {
        real->total <<= 1;
        real->data = (jpointer *) j_realloc(real->data,
                                            sizeof(jpointer) *
                                            real->total);
    }
}

JPtrArray *j_ptr_array_new(void)
{
    return j_ptr_array_new_full(JPtrArrayDefaultSize, NULL);
}

JPtrArray *j_ptr_array_new_full(juint size, JDestroyNotify destroy)
{
    if (J_UNLIKELY(size == 0)) {
        size = JPtrArrayDefaultSize;
    }
    JRealPtrArray *real =
        (JRealPtrArray *) j_malloc(sizeof(JRealPtrArray));
    real->total = size;
    real->free_func = destroy;
    real->len = 0;
    real->data = (jpointer *) j_malloc(sizeof(jpointer) * real->total);
    return (JPtrArray *) real;
}

void j_ptr_array_set_free(JPtrArray * pa, JDestroyNotify destroy)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    real->free_func = destroy;
}

/*
 * Returns the position of the new-added pointer
 */
juint j_ptr_array_append_ptr(JPtrArray * pa, jpointer ptr)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    j_real_ptr_array_may_extend(real, 1);
    real->data[real->len++] = ptr;
    return real->len - 1;
}

void j_ptr_array_append(JPtrArray * pa, ...)
{                               /* Ends with NULL */
    va_list ap;
    va_start(ap, pa);
    jpointer *ptr = va_arg(ap, jpointer);
    while (ptr) {
        j_ptr_array_append_ptr(pa, ptr);
        ptr = va_arg(ap, jpointer);
    }
    va_end(ap);
}

jpointer j_ptr_array_get(JPtrArray * pa, juint index)
{
    if (J_UNLIKELY(index > pa->len)) {
        return NULL;
    }
    return pa->data[index];
}

void j_ptr_array_insert(JPtrArray * pa, jpointer ptr, juint index)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (index >= real->len) {
        j_ptr_array_append_ptr(pa, ptr);
        return;
    }
    j_real_ptr_array_may_extend(real, 1);
    memmove(real->data + index + 1,
            real->data + index, (real->len - index) * sizeof(jpointer));
    real->len++;
    real->data[index] = ptr;
}

jboolean j_ptr_array_remove(JPtrArray * pa, jpointer ptr)
{
    juint i;
    for (i = 0; i < pa->len; i++) {
        if (pa->data[i] == ptr) {
            return j_ptr_array_remove_index(pa, i);
        }
    }
    return FALSE;
}

jboolean j_ptr_array_remove_index(JPtrArray * pa, juint index)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (index >= real->len) {
        return FALSE;
    }
    if (real->free_func) {
        real->free_func(real->data[index]);
    }
    if (J_LIKELY(index != real->len - 1)) {
        memmove(real->data + index, real->data + index + 1,
                sizeof(jpointer) * (real->len - index - 1));
    }
    real->len--;
    return TRUE;
}

jboolean j_ptr_array_remove_index_fast(JPtrArray * pa, juint index)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (index >= real->len) {
        return FALSE;
    }
    if (real->free_func) {
        real->free_func(real->data[index]);
    }
    if (J_LIKELY(index != real->len - 1)) {
        real->data[index] = real->data[real->len - 1];
    }
    real->len--;
    return TRUE;
}

void j_ptr_array_free(JPtrArray * pa, jboolean free_ptr)
{
    JRealPtrArray *real = (JRealPtrArray *) pa;
    JDestroyNotify free_func = real->free_func;
    juint i;
    for (i = 0; i < real->len; i++) {
        if (free_ptr && free_func) {
            free_func(real->data[i]);
        }
    }
    j_free(real->data);
    j_free(real);
}
