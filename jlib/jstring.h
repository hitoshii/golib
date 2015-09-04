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
    char *data;
    unsigned int len;

    /* private */
    unsigned int total;
} JString;

#define j_string_data(s)    (s)->data
#define j_string_len(s)     (s)->len

JString *j_string_new();
JString *j_string_new_with_length(int length);
void j_string_append(JString * string, const char * str);
void j_string_append_len(JString * string, const char * str, unsigned int len);
void j_string_append_c(JString * string, char c);
void j_string_append_printf(JString * string, const char * fmt, ...);

void j_string_preppend(JString * string, const char * str);
void j_string_preppend_len(JString * string, const char * str, unsigned int len);
void j_string_preppend_c(JString * string, char c);
void j_string_preppend_printf(JString * string, const char * fmt, ...);

char *j_string_free(JString * string, boolean free_segment);

/*
 * 从位置pos开始删除len个字节
 * pos表示要移除的起始位置，len为移除的长度，-1表示移除后面所有
 */
void j_string_erase(JString * string, unsigned int pos, int len);

#endif
