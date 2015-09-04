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
#include "jatomic.h"
#include <pthread.h>

static pthread_mutex_t j_atomic_lock = PTHREAD_MUTEX_INITIALIZER;

int j_atomic_int_get(const volatile int * atomic) {
    int value;

    pthread_mutex_lock(&j_atomic_lock);
    value = *atomic;
    pthread_mutex_unlock(&j_atomic_lock);

    return value;
}

void j_atomic_int_set(volatile int * atomic, int value) {
    pthread_mutex_lock(&j_atomic_lock);
    *atomic = value;
    pthread_mutex_unlock(&j_atomic_lock);
}

void j_atomic_int_inc(volatile int * atomic) {
    pthread_mutex_lock(&j_atomic_lock);
    (*atomic)++;
    pthread_mutex_unlock(&j_atomic_lock);
}

boolean j_atomic_int_dec_and_test(volatile int * atomic) {
    boolean is_zero;

    pthread_mutex_lock(&j_atomic_lock);
    is_zero = --(*atomic) == 0;
    pthread_mutex_unlock(&j_atomic_lock);

    return is_zero;
}

boolean j_atomic_int_compare_and_exchange(volatile int * atomic,
        int oldval, int newval) {
    boolean success;

    pthread_mutex_lock(&j_atomic_lock);

    if ((success = (*atomic == oldval))) {
        *atomic = newval;
    }

    pthread_mutex_unlock(&j_atomic_lock);

    return success;
}

int j_atomic_int_add(volatile int * atomic, int val) {
    int oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval + val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned int j_atomic_int_and(volatile unsigned int * atomic, unsigned int val) {
    unsigned int oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval & val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned int j_atomic_int_or(volatile unsigned int * atomic, unsigned int val) {
    unsigned int oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval | val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned int j_atomic_int_xor(volatile unsigned int * atomic, unsigned int val) {
    unsigned int oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval ^ val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}


void * j_atomic_pointer_get(const volatile void *atomic) {
    const volatile void **ptr = (const volatile void **)atomic;
    void * value;

    pthread_mutex_lock(&j_atomic_lock);
    value = (void*)(*ptr);
    pthread_mutex_unlock(&j_atomic_lock);

    return value;
}

void j_atomic_pointer_set(volatile void *atomic, void * newval) {
    volatile void **ptr = (volatile void**)atomic;

    pthread_mutex_lock(&j_atomic_lock);
    *ptr = newval;
    pthread_mutex_unlock(&j_atomic_lock);
}

boolean j_atomic_pointer_compare_and_exchange(volatile void *atomic,
        void * oldval,
        void * newval) {
    volatile void **ptr = (volatile void**)atomic;
    boolean success;

    pthread_mutex_lock(&j_atomic_lock);

    if ((success = (*ptr == oldval))) {
        *ptr = newval;
    }

    pthread_mutex_unlock(&j_atomic_lock);

    return success;
}

signed long j_atomic_pointer_add(volatile void *atomic, signed long val) {
    volatile signed long *ptr = atomic;
    signed long oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval + val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned long j_atomic_pointer_and(volatile void *atomic, unsigned long val) {
    volatile unsigned long *ptr = atomic;
    unsigned long oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval & val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned long j_atomic_pointer_or(volatile void *atomic, unsigned long val) {
    volatile unsigned long *ptr = atomic;
    unsigned long oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval | val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

unsigned long j_atomic_pointer_xor(volatile void *atomic, unsigned long val) {
    volatile unsigned long *ptr = atomic;
    unsigned long oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval ^ val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}
