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
#ifndef __JLIB_OBJECT_H__
#define __JLIB_OBJECT_H__

#include "jtypes.h"

typedef struct _JObject JObject;

typedef void (*JObjectDestroy) (jpointer * obj);

struct _JObject {
    jint ref;
    jpointer priv;
    JObjectDestroy free;
};

#define j_object_get_priv(obj)  (obj)->priv

JObject *j_object_new_proxy(jpointer priv, JObjectDestroy _free);
void j_object_ref(JObject * obj);
void j_object_unref(JObject * obj);


#endif
