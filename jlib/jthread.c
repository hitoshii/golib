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
#include "jthread.h"
#include "jmem.h"
#include "jstrfuncs.h"
#include "jatomic.h"
#include <stdlib.h>
#include <string.h>

struct _JThread {
    JObject parent;
    pthread_t impl;
    JMutex lock;
    boolean joined;

    boolean jlibs;             /* 该线程是否是jlib创建的 */

    JThreadFunc func;
    void * data;
    boolean joinable;

    char *name;
    void * retval;
};

void j_mutex_init(JMutex * mutex) {
    pthread_mutex_init(&mutex->impl, NULL);
}

void j_mutex_clear(JMutex * mutex) {
    pthread_mutex_destroy(&mutex->impl);
}

void j_mutex_lock(JMutex * mutex) {
    pthread_mutex_lock(&mutex->impl);
}

boolean j_mutex_trylock(JMutex * mutex) {
    return pthread_mutex_trylock(&mutex->impl) == 0;
}

void j_mutex_unlock(JMutex * mutex) {
    pthread_mutex_unlock(&mutex->impl);
}

void j_cond_init(JCond * cond) {
    pthread_cond_init(&cond->impl, NULL);
}

void j_cond_clear(JCond * cond) {
    pthread_cond_destroy(&cond->impl);
}

void j_cond_wait(JCond * cond, JMutex * mutex) {
    pthread_cond_wait(&cond->impl, &mutex->impl);
}

boolean j_cond_wait_until(JCond * cond, JMutex * mutex, int64_t end_time) {
    struct timespec ts;
    ts.tv_sec = end_time / 1000000;
    ts.tv_nsec = (end_time % 1000000) * 1000;
    int status = pthread_cond_timedwait(&cond->impl, &mutex->impl, &ts);
    return status == 0;
}

void j_cond_signal(JCond * cond) {
    pthread_cond_signal(&cond->impl);
}

void j_cond_broadcast(JCond * cond) {
    pthread_cond_broadcast(&cond->impl);
}

static inline pthread_key_t *j_private_get_key(JPrivate * priv) {
    pthread_key_t *key =
        (pthread_key_t *) j_atomic_pointer_get(&priv->impl);
    if (J_UNLIKELY(key == NULL)) {
        key = (pthread_key_t *) j_malloc(sizeof(pthread_key_t));
        if (!j_atomic_pointer_compare_and_exchange(&priv->impl, NULL, key)) {
            j_free(key);
            key = priv->impl;
        } else {
            pthread_key_create(key, priv->destroy);
        }
    }
    return key;
}

void * j_private_get(JPrivate * priv) {
    pthread_key_t *key = j_private_get_key(priv);
    return pthread_getspecific(*key);
}

void j_private_set(JPrivate * priv, void * data) {
    pthread_key_t *key = j_private_get_key(priv);
    pthread_setspecific(*key, data);
}

static void j_thread_cleanup(void * data);

J_LOCK_DEFINE_STATIC(j_thread_new);
J_PRIVATE_DEFINE_STATIC(j_thread_specific_private, j_thread_cleanup);


#include <sys/prctl.h>
static inline void j_thread_set_name(const char * name) {
    prctl(PR_SET_NAME, name, 0, 0, 0, 0);
}

static void * thread_func_proxy(void * data) {
    if (data == NULL) {
        return NULL;
    }
    JThread *thread = (JThread *) data;
    j_private_set(&j_thread_specific_private, data);
    J_LOCK(j_thread_new);
    J_UNLOCK(j_thread_new);
    if (thread->name) {
        /* set thread name */
        j_thread_set_name(thread->name);
    }
    thread->retval = thread->func(thread->data);
    return NULL;
}

static inline void j_thread_free(JThread * thread) {
    if (!thread->joined) {
        pthread_detach(thread->impl);
    }
    j_free(thread->name);
    j_mutex_clear(&thread->lock);
}

static void j_thread_destroy(JThread * thread) {
    if (thread->jlibs) {
        j_thread_free(thread);
    }
}

static inline JThread *j_thread_new_internal(const char * name,
        JThreadFunc func,
        void * data) {
    J_LOCK(j_thread_new);
    JThread *thread = (JThread *) j_malloc(sizeof(JThread));
    J_OBJECT_INIT(thread, j_thread_destroy);
    int ret =
        pthread_create(&thread->impl, NULL, thread_func_proxy, thread);
    if (J_UNLIKELY(ret != 0)) {
        j_free(thread);
        return NULL;
    }
    j_mutex_init(&thread->lock);
    thread->name = j_strdup(name);
    thread->joinable = TRUE;
    thread->joined = FALSE;
    thread->func = func;
    thread->data = data;
    thread->jlibs = TRUE;

    j_thread_ref(thread);
    J_UNLOCK(j_thread_new);
    return thread;
}

JThread *j_thread_new(const char * name, JThreadFunc func, void * data) {
    JThread *thread = j_thread_new_internal(name, func, data);
    return thread;
}

JThread *j_thread_try_new(const char * name, JThreadFunc func,
                          void * data) {
    JThread *thread = j_thread_new_internal(name, func, data);
    return thread;

}

static inline void j_thread_join_internal(JThread * thread) {
    j_mutex_lock(&thread->lock);
    if (!thread->joined) {
        pthread_join(thread->impl, NULL);
        thread->joined = TRUE;
    }

    j_mutex_unlock(&thread->lock);

}

void * j_thread_join(JThread * thread) {
    void * retval;

    j_thread_join_internal(thread);
    retval = thread->retval;
    thread->joinable = FALSE;
    j_thread_unref(thread);
    return retval;
}

static void j_thread_cleanup(void * data) {
    j_thread_unref(data);
}


JThread *j_thread_self(void) {
    JThread *thread = j_private_get(&j_thread_specific_private);
    if (thread == NULL) {
        thread = j_malloc(sizeof(JThread));
        J_OBJECT_INIT(thread, NULL);
        thread->jlibs = FALSE;
        j_private_set(&j_thread_specific_private, thread);
    }
    return thread;
}

#include <sched.h>
void j_thread_yield() {
    sched_yield();
}

void j_thread_exit(void * retval) {
    JThread *thread = j_thread_self();
    if (thread == NULL) {
        return;
    }
    thread->retval = retval;
    pthread_exit(NULL);
}

#include "jslist.h"

J_MUTEX_DEFINE_STATIC(j_once_mutex);
J_COND_DEFINE_STATIC(j_once_cond);
static JSList *j_once_init_list = NULL;

boolean j_once_init_enter(volatile void *location) {
    volatile void **value_location = (volatile void**)location;
    boolean need_init = FALSE;
    j_mutex_lock(&j_once_mutex);
    if (j_atomic_pointer_get(value_location) == NULL) {
        if (!j_slist_find(j_once_init_list, (void *) value_location)) {
            need_init = TRUE;
            j_once_init_list =
                j_slist_append(j_once_init_list,
                               (void *) value_location);
        } else {                /* 初始化已经在另外一个线程执行，等待该线程执行完毕 */
            do {
                j_cond_wait(&j_once_cond, &j_once_mutex);
            } while (j_slist_find
                     (j_once_init_list, (void *) value_location));
        }
    }
    j_mutex_unlock(&j_once_mutex);
    return need_init;
}

void j_once_init_leave(volatile void *location, void * result) {
    volatile void **value_location = (volatile void**)location;
    if (j_atomic_pointer_get(value_location) != NULL || result == NULL
            || j_once_init_list == NULL) {
        return;
    }

    j_atomic_pointer_set(value_location, result);
    j_mutex_lock(&j_once_mutex);
    j_once_init_list =
        j_slist_remove(j_once_init_list, (void *) value_location);
    j_cond_broadcast(&j_once_cond);
    j_mutex_unlock(&j_once_mutex);
}
