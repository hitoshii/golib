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
#ifndef __JLIB_BYTE_ARRAY_H__
#define __JLIB_BYTE_ARRAY_H__


typedef struct {
    void *data;
    unsigned int len;

    /* private */
    unsigned int total;
} JByteArray;
#define j_byte_array_get_data(ba)   (ba)->data
#define j_byte_array_get_len(ba)    (ba)->len
#define j_byte_array_get_total(ba)  (ba)->total


JByteArray *j_byte_array_new(void);

void j_byte_array_append(JByteArray * ba, const void *data,
                         unsigned int len);

void *j_byte_array_free(JByteArray * ba, int f);

#endif
