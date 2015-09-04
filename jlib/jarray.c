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
#include "jarray.h"
#include "jmem.h"
#include <string.h>
#include <stdarg.h>


typedef struct {
    uint8_t *data;
    unsigned int len;
    unsigned int total;                /* */
} JRealByteArray;

#define JByteArrayDefaultSize (1024)


static inline void j_byte_array_extend(JRealByteArray * ba) {
    ba->total = ba->total << 1;
    ba->data = j_realloc(ba->data, ba->total);
}

static inline void j_byte_array_may_extend(JRealByteArray * real,
        unsigned int len) {
    while (real->total < len + real->len) {
        j_byte_array_extend(real);
    }
}


JByteArray *j_byte_array_new(void) {
    return j_byte_array_sized_new(JByteArrayDefaultSize);
}

JByteArray *j_byte_array_sized_new(unsigned int size) {
    if (J_UNLIKELY(size == 0)) {
        size = JByteArrayDefaultSize;
    }
    JRealByteArray *real =
        (JRealByteArray *) j_malloc(sizeof(JRealByteArray));
    real->total = size;
    real->len = 0;
    real->data = j_malloc(sizeof(uint8_t) * real->total);
    return (JByteArray *) real;
}

void j_byte_array_append(JByteArray * ba, const uint8_t * data, unsigned int len) {
    if (J_UNLIKELY(len == 0)) {
        return;
    }

    JRealByteArray *array = (JRealByteArray *) ba;
    j_byte_array_may_extend(array, len);

    memcpy(ba->data + ba->len, data, len);
    ba->len += len;
}

void j_byte_array_preppend(JByteArray * ba, const uint8_t * data, unsigned int len) {
    if (J_UNLIKELY(len == 0)) {
        return;
    }
    JRealByteArray *real = (JRealByteArray *) ba;
    j_byte_array_may_extend(real, len);

    memmove(ba->data + len, ba->data, ba->len);
    memcpy(ba->data, data, len);
    ba->len += len;
}

void j_byte_array_clear(JByteArray * ba) {
    ba->len = 0;
}

uint8_t *j_byte_array_free(JByteArray * ba, boolean f) {
    uint8_t *data = j_byte_array_get_data(ba);
    j_free(ba);
    if (f) {
        j_free(data);
        return NULL;
    }
    return data;
}



typedef struct {
    void * *data;
    unsigned int len;
    unsigned int total;
    JDestroyNotify free_func;
} JRealPtrArray;

#define JPtrArrayDefaultSize (32)

static inline void j_real_ptr_array_may_extend(JRealPtrArray * real,
        unsigned int len) {
    while (J_UNLIKELY(real->total < real->len + len)) {
        real->total <<= 1;
        real->data = (void * *) j_realloc(real->data,
                                          sizeof(void *) *
                                          real->total);
    }
}

JPtrArray *j_ptr_array_new(void) {
    return j_ptr_array_new_full(JPtrArrayDefaultSize, NULL);
}

JPtrArray *j_ptr_array_new_full(unsigned int size, JDestroyNotify destroy) {
    if (J_UNLIKELY(size == 0)) {
        size = JPtrArrayDefaultSize;
    }
    JRealPtrArray *real =
        (JRealPtrArray *) j_malloc(sizeof(JRealPtrArray));
    real->total = size;
    real->free_func = destroy;
    real->len = 0;
    real->data = (void * *) j_malloc(sizeof(void *) * real->total);
    return (JPtrArray *) real;
}

void j_ptr_array_set_free(JPtrArray * pa, JDestroyNotify destroy) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    real->free_func = destroy;
}

void j_ptr_array_set_size(JPtrArray * pa, unsigned int size) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (size > real->len) {
        j_real_ptr_array_may_extend(real, (size - real->len));
        int i;
        for (i = real->len; i < size; i++) {
            real->data[i] = NULL;
        }
    } else if (size < real->len) {
        j_ptr_array_remove_range(pa, size, real->len - size);
    }
    real->len = size;
}

void j_ptr_array_remove_range(JPtrArray * pa, unsigned int index, unsigned int length) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    unsigned int n;
    if (real == NULL || index >= real->len || index + length > real->len) {
        return;
    }
    if (real->free_func != NULL) {
        for (n = index; n < index + length; n++) {
            real->free_func(real->data[n]);
        }
    }
    if (index + length != real->len) {
        memmove(&real->data[index],
                &real->data[index + length],
                (real->len - (index + length) * sizeof(void *)));
    }
    real->len -= length;
}

/*
 * Returns the position of the new-added pointer
 */
unsigned int j_ptr_array_append_ptr(JPtrArray * pa, void * ptr) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    j_real_ptr_array_may_extend(real, 1);
    real->data[real->len++] = ptr;
    return real->len - 1;
}

void j_ptr_array_append(JPtrArray * pa, ...) {
    /* Ends with NULL */
    va_list ap;
    va_start(ap, pa);
    void * *ptr = va_arg(ap, void *);
    while (ptr) {
        j_ptr_array_append_ptr(pa, ptr);
        ptr = va_arg(ap, void *);
    }
    va_end(ap);
}

void * j_ptr_array_get(JPtrArray * pa, unsigned int index) {
    if (J_UNLIKELY(index > pa->len)) {
        return NULL;
    }
    return pa->data[index];
}

void * j_ptr_array_find(JPtrArray * pa, JCompareFunc compare,
                        void * user_data) {
    int i;
    for (i = 0; i < pa->len; i++) {
        void * data = pa->data[i];
        if (compare(data, user_data) == 0) {
            return data;
        }
    }
    return NULL;
}

int j_ptr_array_find_index(JPtrArray * pa, JCompareFunc compare,
                           void * user_data) {
    int i;
    for (i = 0; i < pa->len; i++) {
        void * data = pa->data[i];
        if (compare(data, user_data) == 0) {
            return i;
        }
    }
    return -1;
}

void j_ptr_array_insert(JPtrArray * pa, void * ptr, unsigned int index) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (index >= real->len) {
        j_ptr_array_append_ptr(pa, ptr);
        return;
    }
    j_real_ptr_array_may_extend(real, 1);
    memmove(real->data + index + 1,
            real->data + index, (real->len - index) * sizeof(void *));
    real->len++;
    real->data[index] = ptr;
}

boolean j_ptr_array_remove(JPtrArray * pa, void * ptr) {
    unsigned int i;
    for (i = 0; i < pa->len; i++) {
        if (pa->data[i] == ptr) {
            return j_ptr_array_remove_index(pa, i);
        }
    }
    return FALSE;
}

boolean j_ptr_array_remove_index(JPtrArray * pa, unsigned int index) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    if (index >= real->len) {
        return FALSE;
    }
    if (real->free_func) {
        real->free_func(real->data[index]);
    }
    if (J_LIKELY(index != real->len - 1)) {
        memmove(real->data + index, real->data + index + 1,
                sizeof(void *) * (real->len - index - 1));
    }
    real->len--;
    return TRUE;
}

boolean j_ptr_array_remove_index_fast(JPtrArray * pa, unsigned int index) {
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

void j_ptr_array_free(JPtrArray * pa, boolean free_ptr) {
    JRealPtrArray *real = (JRealPtrArray *) pa;
    JDestroyNotify free_func = real->free_func;
    unsigned int i;
    for (i = 0; i < real->len; i++) {
        if (free_ptr && free_func) {
            free_func(real->data[i]);
        }
    }
    j_free(real->data);
    j_free(real);
}

/* 判断数组中是否包含元素data */
boolean j_ptr_array_contains(JPtrArray * array, void * data) {
    JRealPtrArray *real = (JRealPtrArray *) array;
    int i;
    for (i = 0; i < real->len; i++) {
        if (real->data[i] == data) {
            return TRUE;
        }
    }
    return FALSE;
}

/* 插入指针，保证不重复 */
void j_ptr_array_append_ptr_unique(JPtrArray * array, void * data) {
    if (j_ptr_array_contains(array, data)) {
        return;
    }
    j_ptr_array_append_ptr(array, data);
}
