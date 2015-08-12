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

#include "jobject.h"
#include "jatomic.h"
#include "jmem.h"


void j_object_init(JObject * obj, JObjectDestroy _free) {
    obj->ref = 1;
    obj->free = _free;
}

void j_object_ref(JObject * obj) {
    j_atomic_int_inc(&obj->ref);
}

void j_object_unref(JObject * obj) {
    if (j_atomic_int_dec_and_test(&obj->ref)) {
        if (obj->free) {
            obj->free(obj);
        }
        j_free(obj);
    }
}
