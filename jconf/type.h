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
#ifndef __J_CONF_TYPE_H__
#define __J_CONF_TYPE_H__

#include <stdint.h>

typedef struct _JConfData JConfData;

typedef enum {
    J_CONF_DATA_RAW,
    J_CONF_DATA_STRING,
    J_CONF_DATA_INT,
    J_CONF_DATA_TRUE,
    J_CONF_DATA_FALSE,
} JConfDataType;

#define J_CONF_DATA_TRUE_TEXT "True"
#define J_CONF_DATA_FALSE_TEXT "False"

JConfDataType j_conf_data_get_type(JConfData * d);
#define J_CONF_DATA_TYPE(d) (j_conf_data_get_type(d))

#define j_conf_data_is_raw(d)   (J_CONF_DATA_TYPE(d)==J_CONF_DATA_RAW)
#define j_conf_data_is_string(d)    (J_CONF_DATA_TYPE(d)==J_CONF_DATA_STRING)
#define j_conf_data_is_int(d)   (J_CONF_DATA_TYPE(d)==J_CONF_DATA_INT)
#define j_conf_data_is_true(d)  (J_CONF_DATA_TYPE(d)==J_CONF_DATA_TRUE)
#define j_conf_data_is_false(d) (J_CONF_DATA_TYPE(d)==J_CONF_DATA_FALSE)

/*
 * Returns the string value of JConfData.
 * If the type of d is not J_CONF_DATA_RAW or J_CONF_DATA_STRING
 * the return value is undefined
 */
const char *j_conf_data_get_string(JConfData * d);

/*
 * Returns the int value of JConfData.
 * If the type of d is not J_CONF_DATA_INT
 * the return value is undefined
 */
int64_t j_conf_data_get_int(JConfData * d);

/*
 * Creates a new JConfData with specified type
 * If the type is J_CONF_DATA_RAW or J_CONF_DATA_STRING
 * JConfData will just take it. So this function cannot be used on
 * statically allocated string
 */
JConfData *j_conf_data_new(JConfDataType type, ...);


/*
 * Parses a string to create a JConfData
 */
JConfData *j_conf_data_parse(const char *raw);

/*
 * Frees a JConfData, with all memory associated to it
 */
void j_conf_data_free(JConfData * d);


/*
 * Compares two JConfDatas,
 * If d1 is equal to d2, 0 is returned
 */
int j_conf_data_compare(JConfData * d1, JConfData * d2);


#endif
