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
#include "jmem.h"
#include <string.h>
#include <stdlib.h>


void * j_malloc(unsigned int size) {
    if (J_UNLIKELY(size == 0)) {
        return NULL;
    }
    void * ptr = malloc(size);
    return ptr;
}

void * j_malloc0(unsigned int size) {
    if (J_UNLIKELY(size == 0)) {
        return NULL;
    }
    void * ptr = calloc(1, size);
    return ptr;
}

void * j_realloc(void * mem, unsigned int size) {
    if (J_UNLIKELY(size == 0)) {
        j_free(mem);
        return NULL;
    }
    void * ptr = realloc(mem, size);
    return ptr;
}

void j_free(void * ptr) {
    if (J_LIKELY(ptr)) {
        free(ptr);
    }
}

void * j_memdup(const void * data, unsigned int len) {
    void *d = j_malloc(len);
    memcpy(d, data, len);
    return d;
}
