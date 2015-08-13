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
    jchar *filename;            /* 文件名 */
    juint line;                 /* 行号  */
    jint errcode;               /* 错误码，没有错误为0 */
} JConfLoaderInfo;
#define j_conf_loader_info_get_filename(info)   (info)->filename
#define j_conf_loader_info_get_line(info)   (info)->line
#define j_conf_loader_info_get_errcode(info)    (info)->errcode

#define J_CONF_LOADER_ERR_SUCCESS 0
#define J_CONF_LOADER_ERR_INVALID_FILE 1
#define J_CONF_LOADER_ERR_INVALID_KEY 2
#define J_CONF_LOADER_ERR_INVALID_VALUE 3
#define J_CONF_LOADER_ERR_INVALID_STRING 4
#define J_CONF_LOADER_ERR_MISSING_COLON 5
#define J_CONF_LOADER_ERR_MISSING_END 6
#define J_CONF_LOADER_ERR_INVALID_ARRAY 7
#define J_CONF_LOADER_ERR_INVALID_INCLUDE 8


JConfLoader *j_conf_loader_new(void);

#define j_conf_loader_ref(l) J_OBJECT_REF(l)
#define j_conf_loader_unref(l)  J_OBJECT_UNREF(l)

jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path);

jint j_conf_loader_get_errcode(JConfLoader * loader);
jint j_conf_loader_get_line(JConfLoader * loader);
const jchar *j_conf_loader_get_path(JConfLoader * loader);

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
void j_conf_loader_put_integer(JConfLoader * loader, const jchar * name,
                               jint64 integer);
void j_conf_loader_put_string(JConfLoader * loader, const jchar * name,
                              const jchar * string);
void j_conf_loader_put_float(JConfLoader * loader, const jchar * name,
                             jdouble floating);
void j_conf_loader_put_bool(JConfLoader * loader, const jchar * name,
                            jboolean b);
void j_conf_loader_put_null(JConfLoader * loader, const jchar * name);

/**
 * j_conf_loader_get:
 * @loader: JConfLoader
 * @name: the name of variable
 *
 * Gets variable node
 *
 * Returns: a JConfNode or NULL
 */
JConfNode *j_conf_loader_get(JConfLoader * loader, const jchar * name);

#endif
