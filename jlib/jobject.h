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
#ifndef __JLIB_OBJECT_H__
#define __JLIB_OBJECT_H__

#include "jtypes.h"


/*
 * JObject主要就是引用计数，以及“析构”函数
 */

typedef struct _JObject JObject;

typedef void (*JObjectDestroy) (jpointer obj);

struct _JObject {
    jint ref;
    JObjectDestroy free;
};

#define J_OBJECT_INIT(obj, _free) j_object_init((JObject*)obj, (JObjectDestroy)_free)
#define J_OBJECT_REF(obj) j_object_ref((JObject*)obj)
#define J_OBJECT_UNREF(obj) j_object_unref((JObject*)obj)

void j_object_init(JObject * obj, JObjectDestroy _free);
void j_object_ref(JObject * obj);
void j_object_unref(JObject * obj);


#endif
