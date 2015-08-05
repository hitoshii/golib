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
#ifndef __J_CONF_LOADER_H__
#define __J_CONF_LOADER_H__

#include <jio/jio.h>
#include "jconfroot.h"

typedef struct _JConfLoader JConfLoader;

JConfLoader *j_conf_loader_new(void);

#define j_conf_loader_ref(l) J_OBJECT_REF(l)
#define j_conf_loader_unref(l)  J_OBJECT_UNREF(l)

jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path);

JConfRoot *j_conf_loader_get_root(JConfLoader * loader);

void j_conf_loader_put_integer(JConfLoader * loader, const jchar * name,
                               jint64 integer);
void j_conf_loader_put_string(JConfLoader * loader, const jchar * name,
                              const jchar * string);
void j_conf_loader_put_float(JConfLoader * loader, const jchar * name,
                             jdouble floating);
void j_conf_loader_put_bool(JConfLoader * loader, const jchar * name,
                            jboolean b);
void j_conf_loader_put_null(JConfLoader * loader, const jchar * name);

#endif
