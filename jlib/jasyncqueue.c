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
#include "jasyncqueue.h"
#include "jqueue.h"
#include "jmem.h"
#include "jmessage.h"
#include "jatomic.h"


struct _JAsyncQueue {
    JMutex mutex;
    JCond cond;
    JQueue queue;
    JDestroyNotify free_func;
    juint waiting_threads;
    jint ref;
};

JAsyncQueue *j_async_queue_new(void)
{
    return j_async_queue_new_full(NULL);
}

JAsyncQueue *j_async_queue_new_full(JDestroyNotify free_func)
{
    JAsyncQueue *queue = j_malloc(sizeof(JAsyncQueue));
    j_mutex_init(&queue->mutex);
    j_cond_init(&queue->cond);
    j_queue_init(&queue->queue);
    queue->waiting_threads = 0;
    queue->ref = 1;
    queue->free_func = free_func;

    return queue;
}

void j_async_queue_ref(JAsyncQueue * queue)
{
    j_return_if_fail(queue != NULL);
    j_atomic_int_inc(&queue->ref);
}

void j_async_queue_unref(JAsyncQueue * queue)
{
    j_return_if_fail(queue != NULL);
    if (j_atomic_int_dec_and_test(&queue->ref)) {
        j_return_if_fail(queue->waiting_threads == 0);
        j_mutex_clear(&queue->mutex);
        j_cond_clear(&queue->cond);
        if (queue->free_func) {
            j_queue_foreach(&queue->queue, (JFunc) queue->free_func, NULL);
        }
        j_queue_clear(&queue->queue);
        j_free(queue);
    }
}

void j_async_queue_lock(JAsyncQueue * queue)
{
    j_return_if_fail(queue != NULL);
    j_mutex_lock(&queue->mutex);
}

void j_async_queue_unlock(JAsyncQueue * queue)
{
    j_return_if_fail(queue != NULL);
    j_mutex_unlock(&queue->mutex);
}

void j_async_queue_push(JAsyncQueue * queue, jpointer data)
{
    j_return_if_fail(queue != NULL && data != NULL);
    j_mutex_lock(&queue->mutex);
    j_async_queue_push_unlocked(queue, data);
    j_mutex_unlock(&queue->mutex);
}

void j_async_queue_push_unlocked(JAsyncQueue * queue, jpointer data)
{
    j_return_if_fail(queue != NULL && data != NULL);
    j_queue_push_head(&queue->queue, data);
    if (queue->waiting_threads > 0) {
        j_cond_signal(&queue->cond);
    }
}

void j_async_queue_push_sorted(JAsyncQueue * queue, jpointer data,
                               JCompareDataFunc func, jpointer user_data)
{
    j_return_if_fail(queue != NULL);
    j_mutex_lock(&queue->mutex);
    j_async_queue_push_sorted_unlocked(queue, data, func, user_data);
    j_mutex_unlock(&queue->mutex);
}

typedef struct {
    JCompareDataFunc func;
    jpointer user_data;
} SortData;

static jint j_async_queue_invert_compare(jpointer v1, jpointer v2,
                                         SortData * sd)
{
    return -sd->func(v1, v2, sd->user_data);
}

void j_async_queue_push_sorted_unlocked(JAsyncQueue * queue, jpointer data,
                                        JCompareDataFunc func,
                                        jpointer user_data)
{
    j_return_if_fail(queue != NULL);
    SortData sd = { func, user_data };
    j_queue_insert_sorted(&queue->queue, data,
                          (JCompareDataFunc) j_async_queue_invert_compare,
                          &sd);
    if (queue->waiting_threads > 0) {
        j_cond_signal(&queue->cond);
    }
}
