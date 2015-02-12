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
#include "type.h"
#include <stdarg.h>
#include <jlib/jlib.h>

struct _JConfData {
    JConfDataType type;
    union {
        char *str;              /* for J_CONF_DATA_RAW or J_CONF_DATA_STRING */
        int64_t number;         /* for J_CONF_DATA_INT */
    } data;
};

JConfDataType j_conf_data_get_type(JConfData * d)
{
    return d->type;
}

/*
 * Returns the string value of JConfData.
 * If the type of d is not J_CONF_DATA_RAW or J_CONF_DATA_STRING
 * the return value is undefined
 */
const char *j_conf_data_get_string(JConfData * d)
{
    return d->data.str;
}

/*
 * Returns the int value of JConfData.
 * If the type of d is not J_CONF_DATA_INT
 * the return value is undefined
 */
int64_t j_conf_data_get_int(JConfData * d)
{
    return d->data.number;
}

/*
 * Creates a new JConfData with specified type
 * If the type is J_CONF_DATA_RAW or J_CONF_DATA_STRING
 * JConfData will just take it. So this function cannot be used on
 * statically allocated string
 */
JConfData *j_conf_data_new(JConfDataType type, ...)
{
    JConfData *d = (JConfData *) j_malloc(sizeof(JConfData));
    d->type = type;

    va_list vl;
    va_start(vl, type);
    switch (type) {
    case J_CONF_DATA_INT:
        d->data.number = va_arg(vl, int64_t);
        break;
    case J_CONF_DATA_TRUE:
    case J_CONF_DATA_FALSE:
        break;
    case J_CONF_DATA_STRING:
    case J_CONF_DATA_RAW:
        d->data.str = va_arg(vl, char *);
        break;
    }
    va_end(vl);
    return d;
}

static inline char *j_conf_data_str_unescape(char *str)
{
    int flag = 0;
    char *ptr = str;
    while (*ptr) {
        if (flag == 0) {
            if (*ptr == '\\') {
                flag = 1;
            }
        } else {
            if (*ptr == '\\') {
                ptr = j_str_forward(ptr, 1);
            } else if (*ptr == '\"') {
                ptr = j_str_forward(ptr - 1, 1);
            }
            flag = 0;
        }
        ptr++;
    }
    return str;
}

/*
 * Parses a string to create a JConfData
 */
JConfData *j_conf_data_parse(const char *raw)
{
    if (j_strcmp0(raw, J_CONF_DATA_TRUE_TEXT) == 0) {
        return j_conf_data_new(J_CONF_DATA_TRUE);
    } else if (j_strcmp0(raw, J_CONF_DATA_FALSE_TEXT) == 0) {
        return j_conf_data_new(J_CONF_DATA_FALSE);
    }
    int len = j_strlen(raw);
    if (j_str_has_prefix(raw, "\"") && j_str_has_suffix(raw, "\"")
        && len > 1) {
        char *str = j_strdup(raw + 1);
        str[len - 2] = '\0';
        str = j_conf_data_str_unescape(str);
        return j_conf_data_new(J_CONF_DATA_STRING, str);
    } else if (j_str_isint(raw)) {
        return j_conf_data_new(J_CONF_DATA_INT, j_str_toint(raw));
    }
    return j_conf_data_new(J_CONF_DATA_RAW, j_strdup(raw));
}

/*
 * Frees a JConfData, with all memory associated to it
 */
void j_conf_data_free(JConfData * d)
{
    if (d->type == J_CONF_DATA_RAW || d->type == J_CONF_DATA_STRING) {
        j_free(d->data.str);
    }
    j_free(d);
}

/*
 * Compares two JConfDatas,
 * If d1 is equal to d2, 0 is returned
 */
int j_conf_data_compare(JConfData * d1, JConfData * d2)
{
    JConfDataType type1 = j_conf_data_get_type(d1);
    JConfDataType type2 = j_conf_data_get_type(d2);
    if (type1 != type2) {
        return -1;
    }
    if (type1 == J_CONF_DATA_FALSE || type1 == J_CONF_DATA_TRUE) {
        return 0;
    } else if (type1 == J_CONF_DATA_INT &&
               j_conf_data_get_int(d1) == j_conf_data_get_int(d2)) {
        return 0;
    } else if (type1 == J_CONF_DATA_STRING || type1 == J_CONF_DATA_RAW) {
        return j_strcmp0(j_conf_data_get_string(d1),
                         j_conf_data_get_string(d2));
    }
    return 1;
}
