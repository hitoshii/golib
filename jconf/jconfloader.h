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
#ifndef __J_CONF_LOADER_H__
#define __J_CONF_LOADER_H__

#include <jio/jio.h>
#include "jconfroot.h"

typedef struct _JConfLoader JConfLoader;

typedef struct {
    char *filename;            /* 文件名 */
    unsigned int line;                 /* 行号  */
    int errcode;               /* 错误码，没有错误为0 */
} JConfLoaderInfo;
#define j_conf_loader_info_get_filename(info)   (info)->filename
#define j_conf_loader_info_get_line(info)   (info)->line
#define j_conf_loader_info_get_errcode(info)    (info)->errcode

#define J_CONF_LOADER_ERR_SUCCESS 0
#define J_CONF_LOADER_ERR_INVALID_FILE 1
#define J_CONF_LOADER_ERR_INVALID_KEY 2
#define J_CONF_LOADER_ERR_INVALID_VALUE 3
#define J_CONF_LOADER_ERR_INVALID_STRING 4
#define J_CONF_LOADER_ERR_MISSING_VALUE 5
#define J_CONF_LOADER_ERR_MISSING_COLON 6
#define J_CONF_LOADER_ERR_MISSING_END 7
#define J_CONF_LOADER_ERR_INVALID_ARRAY 8
#define J_CONF_LOADER_ERR_INVALID_INCLUDE 9
#define J_CONF_LOADER_ERR_UNKNOWN_VARIABLE 10
#define J_CONF_LOADER_ERR_INVALID_INTEGER 11
#define J_CONF_LOADER_ERR_MISSING_INTEGER 12


JConfLoader *j_conf_loader_new(void);

#define j_conf_loader_ref(l) J_OBJECT_REF(l)
#define j_conf_loader_unref(l)  J_OBJECT_UNREF(l)

void j_conf_loader_allow_unknown_variable(JConfLoader *loader, boolean allow);
boolean j_conf_loader_loads(JConfLoader * loader, const char * path);

int j_conf_loader_get_errcode(JConfLoader * loader);
int j_conf_loader_get_line(JConfLoader * loader);
const char *j_conf_loader_get_path(JConfLoader * loader);

/* 需要释放  */
char *j_conf_loader_build_error_message(JConfLoader *loader);

JConfRoot *j_conf_loader_get_root(JConfLoader * loader);

/**
 * j_conf_loader_put_integer:
 * @loader: JConfLoader
 * @name: variable name
 * @integer: integer value
 *
 * Assigns a integer value to a variable.
 */
void j_conf_loader_put_integer(JConfLoader * loader, const char * name,
                               int64_t integer);
void j_conf_loader_put_string(JConfLoader * loader, const char * name,
                              const char * string);
void j_conf_loader_put_float(JConfLoader * loader, const char * name,
                             double floating);
void j_conf_loader_put_bool(JConfLoader * loader, const char * name,
                            boolean b);
void j_conf_loader_put_null(JConfLoader * loader, const char * name);

/**
 * j_conf_loader_get:
 * @loader: JConfLoader
 * @name: the name of variable
 *
 * Gets variable node
 *
 * Returns: a JConfNode or NULL
 */
JConfNode *j_conf_loader_get(JConfLoader * loader, const char * name);

#endif
