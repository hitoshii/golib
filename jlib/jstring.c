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
#include "jstring.h"
#include "jmem.h"
#include "jstrfuncs.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


static inline void j_string_realloc(JString * string)
{
    string->total *= 2;
    string->data =
        (jchar *) j_realloc(string->data, sizeof(char) * string->total);
}


JString *j_string_new()
{
    JString *string = (JString *) j_malloc(sizeof(JString));
    string->total = 1024;
    string->len = 0;
    string->data = (char *) j_malloc(sizeof(char) * string->total);
    string->data[0] = '\0';
    return string;
}

void j_string_append(JString * string, const jchar * str)
{
    jint len = j_strlen(str);
    j_string_append_len(string, str, len);
}

void j_string_append_len(JString * string, const jchar * str, juint len)
{
    while (string->len + len >= string->total) {
        j_string_realloc(string);
    }
    memcpy(string->data + string->len, str, len);
    string->len += len;
    string->data[string->len] = '\0';
}

void j_string_append_c(JString * string, jchar c)
{
    if (string->len >= string->total - 1) {
        j_string_realloc(string);
    }
    string->data[string->len] = c;
    string->len++;
    string->data[string->len] = '\0';
}

void j_string_append_printf(JString * string, const jchar * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    jchar *buf = j_strdup_vprintf(fmt, ap);
    j_string_append(string, buf);
    j_free(buf);
    va_end(ap);
}

void j_string_preppend_printf(JString * string, const jchar * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    jchar *buf = j_strdup_vprintf(fmt, ap);
    j_string_preppend(string, buf);
    j_free(buf);
    va_end(ap);
}

void j_string_preppend(JString * string, const jchar * str)
{
    juint len = j_strlen(str);
    j_string_preppend_len(string, str, len);
}

void j_string_preppend_len(JString * string, const jchar * str, juint len)
{
    while (string->len + len >= string->total) {
        j_string_realloc(string);
    }
    memmove(string->data + len, string->data, string->len + 1);
    memcpy(string->data, str, len);
    string->len += len;
}

void j_string_preppend_c(JString * string, jchar c)
{
    if (string->len >= string->total - 1) {
        j_string_realloc(string);
    }
    memmove(string->data + 1, string->data, string->len + 1);
    string->len++;
    string->data[0] = c;
}

jchar *j_string_free(JString * string, jboolean free_segment)
{
    jchar *data = string->data;
    j_free(string);
    if (free_segment) {
        j_free(data);
        return NULL;
    }
    return data;
}

/*
 * 从位置pos开始删除len个字节
 * pos表示要移除的起始位置，len为移除的长度，-1表示移除后面所有
 */
void j_string_erase(JString * string, juint pos, jint len)
{
    if (pos >= string->len || len == 0) {
        return;
    }
    if (len < 0) {
        string->len = pos;
    } else {
        int i;
        for (i = pos; i < string->len && i + len < string->len; i++) {
            string->data[i] = string->data[i + len];
        }
        string->len = i;
    }
    string->data[string->len] = '\0';
}
