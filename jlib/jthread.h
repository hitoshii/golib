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

#ifndef __JLIB_THREAD_H__
#define __JLIB_THREAD_H__

#include "jtypes.h"
#include <pthread.h>

typedef struct _JMutext JMutex;

struct _JMutext {
    pthread_mutex_t posix;
};

#define J_MUTEX_DEFINE(name)  JMutex name = {PTHREAD_MUTEX_INITIALIZER}
#define J_MUTEX_DEFINE_STATIC(name) static J_MUTEX_DEFINE(name)

void j_mutex_init(JMutex * mutex);

void j_mutex_clear(JMutex * mutex);

void j_mutex_lock(JMutex * mutex);
jboolean j_mutex_trylock(JMutex * mutex);
void j_mutex_unlock(JMutex * mutex);


typedef struct _JCond JCond;

struct _JCond {
    pthread_cond_t posix;
};

#define J_COND_DEFINE(name) JCond name = {PTHREAD_COND_INITIALIZER}
#define J_COND_DEFINE_STATIC(name)  static J_COND_DEFINE(name)

void j_cond_init(JCond * cond);
void j_cond_clear(JCond * cond);

void j_cond_wait(JCond * cond, JMutex * mutex);
void j_cond_signal(JCond * cond);

typedef struct _JPrivate JPrivate;

struct _JPrivate {
    pthread_key_t *posix;
    JDestroyNotify destroy;
};

#define J_PRIVATE_DEFINE(name, destroy)  JPrivate name = {NULL, destroy}
#define J_PRIVATE_DEFINE_STATIC(name, destroy) \
                    static J_PRIVATE_DEFINE(name, destroy)

jpointer j_private_get(JPrivate * priv);
void j_private_set(JPrivate * priv, jpointer data);

#endif
