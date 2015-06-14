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
    pthread_mutex_t impl;
};

#define J_MUTEX_DEFINE(name)  JMutex name = {PTHREAD_MUTEX_INITIALIZER}
#define J_MUTEX_DEFINE_STATIC(name) static J_MUTEX_DEFINE(name)

#define J_LOCK_NAME(name)   j__ ## name ## _lock
#define J_LOCK_DEFINE(name) J_MUTEX_DEFINE(J_LOCK_NAME(name))
#define J_LOCK_DEFINE_STATIC(name) J_MUTEX_DEFINE_STATIC(J_LOCK_NAME(name))

#define J_LOCK(name)    j_mutex_lock (&J_LOCK_NAME (name))
#define J_UNLOCK(name)  j_mutex_unlock (&J_LOCK_NAME(name))

void j_mutex_init(JMutex * mutex);
void j_mutex_clear(JMutex * mutex);

void j_mutex_lock(JMutex * mutex);
jboolean j_mutex_trylock(JMutex * mutex);
void j_mutex_unlock(JMutex * mutex);


typedef struct _JCond JCond;

struct _JCond {
    pthread_cond_t impl;
};

#define J_COND_DEFINE(name) JCond name = {PTHREAD_COND_INITIALIZER}
#define J_COND_DEFINE_STATIC(name)  static J_COND_DEFINE(name)

void j_cond_init(JCond * cond);
void j_cond_clear(JCond * cond);

void j_cond_wait(JCond * cond, JMutex * mutex);
/*
 * 收到信号返回TRUE，如果是因为end_time超时则返回FALSE
 */
jboolean j_cond_wait_until(JCond * cond, JMutex * mutex, jint64 end_time);
void j_cond_signal(JCond * cond);
void j_cond_broadcast(JCond * cond);

typedef struct _JPrivate JPrivate;

struct _JPrivate {
    pthread_key_t *posix;
    JDestroyNotify destroy;
};

#define J_PRIVATE_DEFINE(name, destroy)  \
                    JPrivate name = {NULL, (JDestroyNotify)destroy}
#define J_PRIVATE_DEFINE_STATIC(name, destroy) \
                    static J_PRIVATE_DEFINE(name, (JDestroyNotify)destroy)

jpointer j_private_get(JPrivate * priv);
void j_private_set(JPrivate * priv, jpointer data);

/*
 * Thread
 */
typedef jpointer(*JThreadFunc) (jpointer data);

typedef struct _JThread JThread;

JThread *j_thread_new(const jchar * name, JThreadFunc func, jpointer data);

void j_thread_unref(JThread * thread);
void j_thread_ref(JThread * thread);

jpointer j_thread_join(JThread * thread);

/*
 * Must call j_thread_exit() from a thread that created by j_thread_new
 */
void j_thread_exit(jpointer retval);

/*
 * Causes the calling thread to voluntarily relinquish the CPU, so that other threads can run
 * This function is often used as a method to make busy wait less evil
 */
void j_thread_yield();

/*
 * If no thread data is available, returns NULL.
 * This can hanppen for the main thread and threads
 * that are not created by JLib.
 */
JThread *j_thread_self(void);


#endif
