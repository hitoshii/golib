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
#include "jmem.h"
#include <string.h>
#include <stdlib.h>


jpointer j_malloc(juint size)
{
    if (J_UNLIKELY(size == 0)) {
        return NULL;
    }
    jpointer ptr = malloc(size);
    return ptr;
}

jpointer j_malloc0(juint size)
{
    if (J_UNLIKELY(size == 0)) {
        return NULL;
    }
    jpointer ptr = calloc(1, size);
    return ptr;
}

jpointer j_realloc(jpointer mem, juint size)
{
    if (J_UNLIKELY(size == 0)) {
        j_free(mem);
        return NULL;
    }
    jpointer ptr = realloc(mem, size);
    return ptr;
}

void j_free(jpointer ptr)
{
    if (J_LIKELY(ptr)) {
        free(ptr);
    }
}

void *j_memdup(const void *data, unsigned int len)
{
    void *d = j_malloc(len);
    memcpy(d, data, len);
    return d;
}
