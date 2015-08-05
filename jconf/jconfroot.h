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

typedef struct JConfObject JConfRoot;

JConfRoot *j_conf_root_new(void);

#define j_conf_root_ref(r) J_OBJECT_REF(r)
#define j_conf_root_unref(r) J_OBJECT_UNREF(r)

#define j_conf_root_set(root, name, node) j_conf_object_set((JConfObject*)root, name, node)
#define j_conf_root_set_take(root, name, node) j_conf_object_set_take((JConfObject*)root, name, node)
#define j_conf_root_set_integer(root, name, integer) j_conf_object_set_integer((JConfObject*)root, name, integer)
#define j_conf_root_set_string(root, name, string) j_conf_object_set_string((JConfObject*)root, name, string)
#define j_conf_root_set_float(root, name, floating) j_conf_object_set_float((JConfObject*)root, name, floating)
#define j_conf_root_set_bool(root, name, b) j_conf_object_set_bool((JConfObject*)root, name, b)
#define j_conf_root_set_null(root, name) j_conf_object_set_null((JConfObject*)root, name)
#define j_conf_root_get(root, name) j_conf_object_get((JConfObject*)root, name)
#define j_conf_root_get_keys(root) j_conf_object_get_keys((JConfObject*)root)

#endif
