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
#ifndef __J_LIB_STRING_H__
#define __J_LIB_STRING_H__

#include "jtypes.h"

typedef struct {
    jchar *data;
    juint len;

    /* private */
    juint total;
} JString;

#define j_string_data(s)    (s)->data
#define j_string_len(s)     (s)->len

JString *j_string_new();
JString *j_string_new_with_length(jint length);
void j_string_append(JString * string, const jchar * str);
void j_string_append_len(JString * string, const jchar * str, juint len);
void j_string_append_c(JString * string, jchar c);
void j_string_append_printf(JString * string, const jchar * fmt, ...);

void j_string_preppend(JString * string, const jchar * str);
void j_string_preppend_len(JString * string, const jchar * str, juint len);
void j_string_preppend_c(JString * string, jchar c);
void j_string_preppend_printf(JString * string, const jchar * fmt, ...);

jchar *j_string_free(JString * string, jboolean free_segment);

/*
 * 从位置pos开始删除len个字节
 * pos表示要移除的起始位置，len为移除的长度，-1表示移除后面所有
 */
void j_string_erase(JString * string, juint pos, jint len);

#endif
