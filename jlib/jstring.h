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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#ifndef __J_LIB_STRING_H__
#define __J_LIB_STRING_H__

#include "jtypes.h"

typedef struct {
    jchar *data;
    juint len;

    /* private */
    juint total;
} JString;


JString *j_string_new();
void j_string_append(JString * string, const jchar * str);
void j_string_append_len(JString * string, const jchar * str, juint len);
void j_string_append_c(JString * string, jchar c);
void j_string_append_printf(JString * string, const jchar * fmt, ...);
jchar *j_string_free(JString * string, jboolean free_segment);



#endif
