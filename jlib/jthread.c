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
#include "jthread.h"
#include "jmem.h"
#include "jatomic.h"
#include <stdlib.h>

void j_mutex_init(JMutex * mutex)
{
    pthread_mutex_init(&mutex->posix, NULL);
}

void j_mutex_clear(JMutex * mutex)
{
    pthread_mutex_destroy(&mutex->posix);
}

void j_mutex_lock(JMutex * mutex)
{
    pthread_mutex_lock(&mutex->posix);
}

jboolean j_mutex_trylock(JMutex * mutex)
{
    return pthread_mutex_trylock(&mutex->posix) == 0;
}

void j_mutex_unlock(JMutex * mutex)
{
    pthread_mutex_unlock(&mutex->posix);
}

void j_cond_init(JCond * cond)
{
    pthread_cond_init(&cond->posix, NULL);
}

void j_cond_clear(JCond * cond)
{
    pthread_cond_destroy(&cond->posix);
}

void j_cond_wait(JCond * cond, JMutex * mutex)
{
    pthread_cond_wait(&cond->posix, &mutex->posix);
}

void j_cond_signal(JCond * cond)
{
    pthread_cond_signal(&cond->posix);
}

static inline pthread_key_t *j_private_get_key(JPrivate * priv)
{
    pthread_key_t *key =
        (pthread_key_t *) j_atomic_pointer_get(&priv->posix);
    if (J_UNLIKELY(key == NULL)) {
        key = (pthread_key_t *) j_malloc(sizeof(pthread_key_t));
        if (!j_atomic_pointer_compare_and_exchange
            (&priv->posix, NULL, key)) {
            j_free(key);
            key = priv->posix;
        } else {
            pthread_key_create(key, priv->destroy);
        }
    }
    return key;
}

jpointer j_private_get(JPrivate * priv)
{
    pthread_key_t *key = j_private_get_key(priv);
    return pthread_getspecific(*key);
}

void j_private_set(JPrivate * priv, jpointer data)
{
    pthread_key_t *key = j_private_get_key(priv);
    pthread_setspecific(*key, data);
}
