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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "jasyncqueue.h"
#include "jqueue.h"
#include "jmem.h"
#include "jmessage.h"
#include "jatomic.h"
#include "jmain.h"


struct _JAsyncQueue {
    JObject parent;
    JMutex mutex;
    JCond cond;
    JQueue queue;
    JDestroyNotify free_func;
    juint waiting_threads;
};

#define ASYNC_QUEUE_LOCK(q) j_mutex_lock(&(q)->mutex)
#define ASYNC_QUEUE_UNLOCK(q) j_mutex_unlock(&(q)->mutex)

static void j_async_queue_free(JAsyncQueue * queue);

JAsyncQueue *j_async_queue_new(void)
{
    return j_async_queue_new_full(NULL);
}

JAsyncQueue *j_async_queue_new_full(JDestroyNotify free_func)
{
    JAsyncQueue *queue = j_malloc(sizeof(JAsyncQueue));
    J_OBJECT_INIT(queue, j_async_queue_free);
    j_mutex_init(&queue->mutex);
    j_cond_init(&queue->cond);
    j_queue_init(&queue->queue);
    queue->waiting_threads = 0;
    queue->free_func = free_func;

    return queue;
}

static void j_async_queue_free(JAsyncQueue * queue)
{
    j_return_if_fail(queue->waiting_threads == 0);
    j_mutex_clear(&queue->mutex);
    j_cond_clear(&queue->cond);
    if (queue->free_func) {
        j_queue_foreach(&queue->queue, (JFunc) queue->free_func, NULL);
    }
    j_queue_clear(&queue->queue);
}

void j_async_queue_lock(JAsyncQueue * queue)
{
    ASYNC_QUEUE_LOCK(queue);
}

void j_async_queue_unlock(JAsyncQueue * queue)
{
    ASYNC_QUEUE_UNLOCK(queue);
}

void j_async_queue_push(JAsyncQueue * queue, jpointer data)
{
    j_return_if_fail(data != NULL);
    ASYNC_QUEUE_LOCK(queue);
    j_async_queue_push_unlocked(queue, data);
    ASYNC_QUEUE_UNLOCK(queue);
}

void j_async_queue_push_unlocked(JAsyncQueue * queue, jpointer data)
{
    j_return_if_fail(data != NULL);
    j_queue_push_head(&queue->queue, data);
    if (queue->waiting_threads > 0) {
        j_cond_signal(&queue->cond);
    }
}

void j_async_queue_push_sorted(JAsyncQueue * queue, jpointer data,
                               JCompareDataFunc func, jpointer user_data)
{
    ASYNC_QUEUE_LOCK(queue);
    j_async_queue_push_sorted_unlocked(queue, data, func, user_data);
    ASYNC_QUEUE_UNLOCK(queue);
}

void j_async_queue_push_sorted_unlocked(JAsyncQueue * queue, jpointer data,
                                        JCompareDataFunc func,
                                        jpointer user_data)
{
    j_queue_insert_sorted(&queue->queue, data, func, user_data);
    if (queue->waiting_threads > 0) {
        j_cond_signal(&queue->cond);
    }
}

static jpointer j_async_queue_pop_internal_unlocked(JAsyncQueue * queue,
                                                    jboolean wait,
                                                    jint64 end_time)
{
    if (!j_queue_peek_tail_link(&queue->queue) && wait) {
        /* 如果队列的最后一个元素为空，且需要等待 */
        queue->waiting_threads++;
        while (!j_queue_peek_tail_link(&queue->queue)) {
            if (end_time == -1) {
                j_cond_wait(&queue->cond, &queue->mutex);
            } else {
                if (!j_cond_wait_until
                    (&queue->cond, &queue->mutex, end_time)) {
                    break;
                }
            }
        }
        queue->waiting_threads--;
    }
    jpointer retval = j_queue_pop_tail(&queue->queue);
    return retval;
}

jpointer j_async_queue_pop(JAsyncQueue * queue)
{
    ASYNC_QUEUE_LOCK(queue);
    jpointer retval = j_async_queue_pop_internal_unlocked(queue, TRUE, -1);
    ASYNC_QUEUE_UNLOCK(queue);
    return retval;
}

jpointer j_async_queue_pop_unlocked(JAsyncQueue * queue)
{
    return j_async_queue_pop_internal_unlocked(queue, TRUE, -1);
}

jpointer j_async_queue_try_pop(JAsyncQueue * queue)
{
    ASYNC_QUEUE_LOCK(queue);
    jpointer retval =
        j_async_queue_pop_internal_unlocked(queue, FALSE, -1);
    ASYNC_QUEUE_UNLOCK(queue);
    return retval;
}

jpointer j_async_queue_try_pop_unlocked(JAsyncQueue * queue)
{
    return j_async_queue_pop_internal_unlocked(queue, FALSE, -1);
}

jpointer j_async_queue_timeout_pop(JAsyncQueue * queue, juint64 timeout)
{
    ASYNC_QUEUE_LOCK(queue);
    jpointer retval = j_async_queue_timeout_pop_unlocked(queue, timeout);
    ASYNC_QUEUE_UNLOCK(queue);
    return retval;
}

jpointer j_async_queue_timeout_pop_unlocked(JAsyncQueue * queue,
                                            juint64 timeout)
{
    jint64 end_time = j_get_monotonic_time() + timeout;
    return j_async_queue_pop_internal_unlocked(queue, TRUE, end_time);
}

jint j_async_queue_length(JAsyncQueue * queue)
{
    ASYNC_QUEUE_LOCK(queue);
    jint retval = queue->queue.length - queue->waiting_threads;
    ASYNC_QUEUE_UNLOCK(queue);
    return retval;
}

jint j_async_queue_length_unlocked(JAsyncQueue * queue)
{
    return queue->queue.length - queue->waiting_threads;
}

void j_async_queue_sort(JAsyncQueue * queue, JCompareDataFunc func,
                        jpointer user_data)
{
    ASYNC_QUEUE_LOCK(queue);
    j_queue_sort(&queue->queue, func, user_data);
    ASYNC_QUEUE_UNLOCK(queue);
}

void j_async_queue_sort_unlocked(JAsyncQueue * queue,
                                 JCompareDataFunc func, jpointer user_data)
{
    j_queue_sort(&queue->queue, func, user_data);
}

JMutex *j_async_queue_get_mutex(JAsyncQueue * queue)
{
    return &queue->mutex;
}
