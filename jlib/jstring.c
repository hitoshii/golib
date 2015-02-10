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
#include "jstring.h"
#include "jmem.h"


static inline void j_string_realloc(JString * string)
{
    string->total *= 2;
    string->data =
        (char *) j_realloc(string->data, sizeof(char) * string->total);
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

void j_string_append(JString * string, const char *str)
{
    while (*str) {
        j_string_append_c(string, *str);
        str++;
    }
}

void j_string_append_c(JString * string, char c)
{
    if (string->len >= string->total - 1) {
        j_string_realloc(string);
    }
    string->data[string->len] = c;
    string->len++;
    string->data[string->len] = '\0';
}

char *j_string_free(JString * string, int free_segment)
{
    char *data = string->data;
    j_free(string);
    if (free_segment) {
        j_free(data);
        return NULL;
    }
    return data;
}
