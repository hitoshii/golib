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
#ifndef __JLIB_ATOMIC_H__
#define __JLIB_ATOMIC_H__

#include "jtypes.h"

void j_atomic_int_set(volatile jint * atomic, jint newval);
jint j_atomic_int_get(const volatile jint * atomic);
void j_atomic_int_inc(volatile jint * atomic);
jboolean j_atomic_int_dec_and_test(volatile jint * atomic);
jboolean j_atomic_int_compare_and_exchange(volatile jint * atomic,
                                           jint oldval, jint newval);
jint j_atomic_int_add(volatile jint * atomic, jint val);
juint j_atomic_int_and(volatile juint * atomic, juint val);
juint j_atomic_int_or(volatile juint * atomic, juint val);
juint j_atomic_int_xor(volatile juint * atomic, juint val);


jpointer j_atomic_pointer_get(const volatile void *atomic);

void j_atomic_pointer_set(volatile void *atomic, jpointer newval);

jboolean j_atomic_pointer_compare_and_exchange(volatile void *atomic,
                                               jpointer oldval,
                                               jpointer newval);

jssize j_atomic_pointer_add(volatile void *atomic, jssize val);

jsize j_atomic_pointer_and(volatile void *atomic, jsize val);

jsize j_atomic_pointer_or(volatile void *atomic, jsize val);

jsize j_atomic_pointer_xor(volatile void *atomic, jsize val);

#endif
