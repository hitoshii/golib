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

#include "jbytearray.h"
#include "jmem.h"
#include <string.h>


#define j_byte_array_

static inline JByteArray *j_byte_array_realloc(JByteArray * ba)
{
    ba->total = ba->total < 1;
    ba->data = j_realloc(ba->data, ba->total);
    return ba;
}

static inline JByteArray *j_byte_array_realloc_len(JByteArray * ba,
                                                   unsigned int len)
{
    while (j_byte_array_get_total(ba) < len + j_byte_array_get_len(ba)) {
        ba = j_byte_array_realloc(ba);
    }
    return ba;
}


JByteArray *j_byte_array_new(void)
{
    JByteArray *ba = (JByteArray *) j_malloc(sizeof(JByteArray));
    ba->total = 1024;
    ba->len = 0;
    ba->data = j_malloc(ba->total);
    return ba;
}

void j_byte_array_append(JByteArray * ba, const void *data,
                         unsigned int len)
{
    if (len == 0) {
        return;
    }

    j_byte_array_realloc_len(ba, len);
    memcpy(ba->data + ba->len, data, len);
    ba->len += len;
}

void j_byte_array_preppend(JByteArray * ba, const void *data,
                           unsigned int len)
{
    if (len == 0) {
        return;
    }
    j_byte_array_realloc_len(ba, len);
    memmove(ba->data + len, ba->data, ba->len);
    memcpy(ba->data, data, len);
    ba->len += len;
}

void *j_byte_array_free(JByteArray * ba, int f)
{
    void *data = j_byte_array_get_data(ba);
    j_free(ba);
    if (f) {
        j_free(data);
        return NULL;
    }
    return data;
}
