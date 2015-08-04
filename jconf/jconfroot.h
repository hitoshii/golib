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
#ifndef __J_CONF_ROOT_H__
#define __J_CONF_ROOT_H__

#include "jconfnode.h"

typedef struct _JConfRoot JConfRoot;

JConfRoot *j_conf_root_new(void);

#define j_conf_root_ref(r) J_OBJECT_REF(r)
#define j_conf_root_unref(r) J_OBJECT_UNREF(r)

void j_conf_root_set(JConfRoot * root, const jchar * name,
                     JConfNode * node);
void j_conf_root_set_integer(JConfRoot * root, const jchar * name,
                             jint64 integer);
void j_conf_root_set_string(JConfRoot * root, const jchar * name,
                            const jchar * string);
void j_conf_root_set_float(JConfRoot * root, const jchar * name,
                           jdouble floating);
void j_conf_root_set_bool(JConfRoot * root, const jchar * name,
                          jboolean b);
void j_conf_root_set_null(JConfRoot * root, const jchar * name);
JConfNode *j_conf_root_get(JConfRoot * root, const jchar * name);

#endif
