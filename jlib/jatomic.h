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
#ifndef __JLIB_ATOMIC_H__
#define __JLIB_ATOMIC_H__

#include "jtypes.h"

void j_atomic_int_set(volatile int * atomic, int newval);
int j_atomic_int_get(const volatile int * atomic);
void j_atomic_int_inc(volatile int * atomic);
boolean j_atomic_int_dec_and_test(volatile int * atomic);
boolean j_atomic_int_compare_and_exchange(volatile int * atomic,
        int oldval, int newval);
int j_atomic_int_add(volatile int * atomic, int val);
unsigned int j_atomic_int_and(volatile unsigned int * atomic, unsigned int val);
unsigned int j_atomic_int_or(volatile unsigned int * atomic, unsigned int val);
unsigned int j_atomic_int_xor(volatile unsigned int * atomic, unsigned int val);


void * j_atomic_pointer_get(const volatile void *atomic);

void j_atomic_pointer_set(volatile void *atomic, void * newval);

boolean j_atomic_pointer_compare_and_exchange(volatile void *atomic,
        void * oldval,
        void * newval);

signed long j_atomic_pointer_add(volatile void *atomic, signed long val);

unsigned long j_atomic_pointer_and(volatile void *atomic, unsigned long val);

unsigned long j_atomic_pointer_or(volatile void *atomic, unsigned long val);

unsigned long j_atomic_pointer_xor(volatile void *atomic, unsigned long val);

#endif
