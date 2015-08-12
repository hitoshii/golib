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

jint j_atomic_int_get(const volatile jint * atomic) {
    jint value;

    pthread_mutex_lock(&j_atomic_lock);
    value = *atomic;
    pthread_mutex_unlock(&j_atomic_lock);

    return value;
}

void j_atomic_int_set(volatile jint * atomic, jint value) {
    pthread_mutex_lock(&j_atomic_lock);
    *atomic = value;
    pthread_mutex_unlock(&j_atomic_lock);
}

void j_atomic_int_inc(volatile jint * atomic) {
    pthread_mutex_lock(&j_atomic_lock);
    (*atomic)++;
    pthread_mutex_unlock(&j_atomic_lock);
}

jboolean j_atomic_int_dec_and_test(volatile jint * atomic) {
    jboolean is_zero;

    pthread_mutex_lock(&j_atomic_lock);
    is_zero = --(*atomic) == 0;
    pthread_mutex_unlock(&j_atomic_lock);

    return is_zero;
}

jboolean j_atomic_int_compare_and_exchange(volatile jint * atomic,
        jint oldval, jint newval) {
    jboolean success;

    pthread_mutex_lock(&j_atomic_lock);

    if ((success = (*atomic == oldval))) {
        *atomic = newval;
    }

    pthread_mutex_unlock(&j_atomic_lock);

    return success;
}

jint j_atomic_int_add(volatile jint * atomic, jint val) {
    jint oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval + val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

juint j_atomic_int_and(volatile juint * atomic, juint val) {
    juint oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval & val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

juint j_atomic_int_or(volatile juint * atomic, juint val) {
    juint oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval | val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

juint j_atomic_int_xor(volatile juint * atomic, juint val) {
    juint oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *atomic;
    *atomic = oldval ^ val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}


jpointer j_atomic_pointer_get(const volatile void *atomic) {
    const volatile jpointer *ptr = atomic;
    jpointer value;

    pthread_mutex_lock(&j_atomic_lock);
    value = *ptr;
    pthread_mutex_unlock(&j_atomic_lock);

    return value;
}

void j_atomic_pointer_set(volatile void *atomic, jpointer newval) {
    volatile jpointer *ptr = atomic;

    pthread_mutex_lock(&j_atomic_lock);
    *ptr = newval;
    pthread_mutex_unlock(&j_atomic_lock);
}

jboolean j_atomic_pointer_compare_and_exchange(volatile void *atomic,
        jpointer oldval,
        jpointer newval) {
    volatile jpointer *ptr = atomic;
    jboolean success;

    pthread_mutex_lock(&j_atomic_lock);

    if ((success = (*ptr == oldval))) {
        *ptr = newval;
    }

    pthread_mutex_unlock(&j_atomic_lock);

    return success;
}

jssize j_atomic_pointer_add(volatile void *atomic, jssize val) {
    volatile jssize *ptr = atomic;
    jssize oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval + val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

jsize j_atomic_pointer_and(volatile void *atomic, jsize val) {
    volatile jsize *ptr = atomic;
    jsize oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval & val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

jsize j_atomic_pointer_or(volatile void *atomic, jsize val) {
    volatile jsize *ptr = atomic;
    jsize oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval | val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}

jsize j_atomic_pointer_xor(volatile void *atomic, jsize val) {
    volatile jsize *ptr = atomic;
    jsize oldval;

    pthread_mutex_lock(&j_atomic_lock);
    oldval = *ptr;
    *ptr = oldval ^ val;
    pthread_mutex_unlock(&j_atomic_lock);

    return oldval;
}
