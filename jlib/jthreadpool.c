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


/* 启动线程池的线程 */
static jboolean j_thread_pool_start_thread(JRealThreadPool * pool,
                                           JError ** error);
/* 线程代理 */
static jpointer j_thread_pool_thread_proxy(jpointer data);

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
    return NULL;
}
