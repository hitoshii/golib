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
#include "jthreadpool.h"
#include "jasyncqueue.h"
#include "jmessage.h"
#include "jthread.h"
#include "jmem.h"


typedef struct {
    JThreadPool pool;
    JAsyncQueue *queue;
    JCond cond;
    jint max_threads;           /* 最大线程数 */
    jint num_threads;           /* 当前线程数 */
    jboolean running;           /* 线程池正在执行 */
    jboolean immediate;         /* 线程池是否立即关闭 */
    jboolean waiting;           /* 调用函数是否等待任务结束 */

    JCompareDataFunc sort_func;
    jpointer sort_user_data;
} JRealThreadPool;


/* 未被使用的线程都等待unused_thread_queue */
static JAsyncQueue *unused_thread_queue = NULL;


static void j_thread_pool_free_internal(JRealThreadPool * pool);


/* 启动线程池的线程 */
static jboolean j_thread_pool_start_thread(JRealThreadPool * pool,
                                           JError ** error);
/* 线程代理 */
static jpointer j_thread_pool_thread_proxy(jpointer data);
/* 如果该函数返回NULL，则线程会进入公共线程池 */
static jpointer j_thread_pool_wait_for_new_task(JRealThreadPool * pool);
/* 为线程寻找一个合适的线程池 */
static JRealThreadPool *j_thread_pool_wait_for_new_pool(void);

/* 唤醒线程池中其他所有的线程，并结束线程池 */
static void j_thread_pool_wakeup_and_stop_all(JRealThreadPool * pool);

/*
 * 创建一个线程池
 * @max_threads: 最大的线程数量，-1表示没有限制
 * @exclusive: 线程池是否与其他线程池共享线程
 */
JThreadPool *j_thread_pool_new(JFunc func, jpointer user_data,
                               jint max_threads, jboolean exclusive,
                               JError ** error)
{
    j_return_val_if_fail(func != NULL && max_threads >= -1, NULL);
    j_return_val_if_fail(!exclusive || max_threads != -1, NULL);

    J_LOCK_DEFINE_STATIC(init);
    JRealThreadPool *pool = j_malloc(sizeof(JRealThreadPool));
    pool->pool.func = func;
    pool->pool.user_data = user_data;
    pool->pool.exclusive = exclusive;
    pool->queue = j_async_queue_new();
    j_cond_init(&pool->cond);
    pool->max_threads = max_threads;
    pool->num_threads = 0;
    pool->running = TRUE;
    pool->immediate = FALSE;
    pool->waiting = FALSE;
    pool->sort_func = NULL;
    pool->sort_user_data = NULL;

    J_LOCK(init);
    if (!unused_thread_queue) {
        unused_thread_queue = j_async_queue_new();
    }
    J_UNLOCK(init);

    if (pool->pool.exclusive) {
        j_async_queue_lock(pool->queue);
        while (pool->num_threads < pool->max_threads) {
            JError *local_error = NULL;
            if (!j_thread_pool_start_thread(pool, &local_error)) {
                j_propagate_error(error, local_error);
                break;
            }
        }
        j_async_queue_unlock(pool->queue);
    }
    return (JThreadPool *) pool;
}


static jboolean j_thread_pool_start_thread(JRealThreadPool * pool,
                                           JError ** error)
{
    jboolean success = FALSE;

    if (pool->num_threads >= pool->max_threads && pool->max_threads != -1) {
        /* 已经达到最大线程数量 */
        return TRUE;
    }

    j_async_queue_lock(unused_thread_queue);
    if (j_async_queue_length_unlocked(unused_thread_queue) < 0) {
        /* JAsyncQueue的长度是实际长度减去正在等待的线程数量，因此可能小于0 */
        j_async_queue_push_unlocked(unused_thread_queue, pool);
        success = TRUE;
    }

    j_async_queue_unlock(unused_thread_queue);

    if (!success) {
        JThread *thread =
            j_thread_try_new("pool", j_thread_pool_thread_proxy, pool,
                             error);
        if (thread == NULL) {
            return FALSE;
        }
        j_thread_unref(thread);
    }

    pool->num_threads++;
    return TRUE;
}

static jpointer j_thread_pool_thread_proxy(jpointer data)
{
    JRealThreadPool *pool = (JRealThreadPool *) data;

    j_async_queue_lock(pool->queue);

    while (TRUE) {
        jpointer task = j_thread_pool_wait_for_new_task(pool);
        if (task) {
            if (pool->running || !pool->immediate) {
                /* 收到一个任务，并且线程池还是活跃的，执行任务 */
                j_async_queue_unlock(pool->queue);
                pool->pool.func(task, pool->pool.user_data);
                j_async_queue_lock(pool->queue);
                continue;
            }
        }
        /* "善后"工作 */
        jboolean free_pool = FALSE;
        pool->num_threads--;
        if (!pool->running) {
            if (!pool->waiting) {
                if (pool->num_threads == 0) {
                    /* 如果线程池已经不活跃，没有线程在等待线程池结束，
                     * 而且这是最后的线程，则释放该线程池 */
                    free_pool = TRUE;
                } else {
                    /* 如果线程池已经不再活跃，没有线程在等待
                     * 但此线程不是最后的线程，队列里也没有剩余的任务，
                     * 则唤醒其他线程 */
                    if (j_async_queue_length_unlocked(pool->queue) ==
                        -pool->num_threads) {
                        j_thread_pool_wakeup_and_stop_all(pool);
                    }
                }
            } else if (pool->immediate ||
                       j_async_queue_length_unlocked(pool->queue) <= 0) {
                /* 如果线程池已经不活跃，但是有线程在等待它结束，
                 * 没有额外的任务或者线程池被要求立即中止
                 * 通知在等待的线程该线程的状态已经改变
                 */
                j_cond_broadcast(&pool->cond);
            }
        }
        j_async_queue_unlock(pool->queue);
        if (free_pool) {
            j_thread_pool_free_internal(pool);
        }
        if ((pool = j_thread_pool_wait_for_new_pool()) == NULL) {
            break;
        }

        j_async_queue_lock(pool->queue);
    }
    return NULL;
}

static jpointer j_thread_pool_wait_for_new_task(JRealThreadPool * pool)
{
    jpointer task = NULL;
    if (pool->running || (pool->immediate == FALSE &&
                          j_async_queue_length_unlocked(pool->queue) >
                          0)) {
        /* 该线程池是活跃的 */
        if (pool->num_threads > pool->max_threads
            && pool->max_threads != -1) {
            /* 线程过多 */
            j_debug("superfluous thread %p in pool %p.", j_thread_self(),
                    pool);
        } else if (pool->pool.exclusive) {
            /* 如果线程池是独占的，那么该线程池创建的线程永远都只为该线程池服务 */
            task = j_async_queue_pop_unlocked(pool->queue);
        } else {
            /* 如果线程池不是独占的，那么该线程池创建的线程等待500毫秒后进入公共线程池 */
            task = j_async_queue_timeout_pop_unlocked(pool->queue, 500000);
        }
    } else {
        j_debug("pool %p not active, thread %p will go to global pool"
                "(running: %s, immediate: %s, len: %d)",
                pool, j_thread_self(), pool->running ? "true" : "false",
                pool->immediate ? "true" : "false",
                j_async_queue_length_unlocked(pool->queue));
    }
    return task;
}

static void j_thread_pool_wakeup_and_stop_all(JRealThreadPool * pool)
{
    j_return_if_fail(pool->running == FALSE);
    j_return_if_fail(pool->num_threads != 0);
    pool->immediate = TRUE;

    jint i;
    /* 给线程发送1后，线程会第一次会发现线程已经不再活跃，不会执行任何任务
     * 这时它会进入"善后"工作
     */
    for (i = 0; i < pool->num_threads; i++) {
        j_async_queue_push_unlocked(pool->queue, JUINT_TO_JPOINTER(1));
    }
}

static void j_thread_pool_free_internal(JRealThreadPool * pool)
{
    j_return_if_fail(pool->running == FALSE);
    j_return_if_fail(pool->num_threads == 0);

    j_async_queue_unref(pool->queue);
    j_cond_clear(&pool->cond);

    j_free(pool);
}

static JRealThreadPool *j_thread_pool_wait_for_new_pool(void)
{
    /* TODO */
    return NULL;
}
