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
#include "jthreadpool.h"
#include "jasyncqueue.h"
#include "jmessage.h"
#include "jthread.h"
#include "jatomic.h"
#include "jtimer.h"
#include "jmem.h"


typedef struct {
    JThreadPool pool;
    JAsyncQueue *queue;
    JCond cond;
    int max_threads;           /* 最大线程数 */
    int num_threads;           /* 当前线程数 */
    boolean running;           /* 线程池正在执行 */
    boolean immediate;         /* 线程池是否立即关闭 */
    boolean waiting;           /* 调用函数是否等待线程池结束 */

    JCompareDataFunc sort_func;
    void * sort_user_data;
} JRealThreadPool;


#define j_real_thread_pool_lock(pool) j_async_queue_lock((pool)->queue)
#define j_real_thread_pool_unlock(pool) j_async_queue_unlock((pool)->queue)

/*
 * wakeup_thread_marker可以是任意的非空指针，用来唤醒队列中的等待线程
 */
static const void * wakeup_thread_marker =
    (void *) & j_thread_pool_new;
static int wakeup_thread_serial = 0;

/* 未被使用的线程都等待unused_thread_queue */
static JAsyncQueue *unused_thread_queue = NULL;
static int unused_threads = 0;
static int max_unused_threads = 2;
static int kill_unused_threads = 0;
static int max_idle_time = 15 * 1000;


static void j_thread_pool_free_internal(JRealThreadPool * pool);


/* 启动线程池的线程 */
static boolean j_thread_pool_start_thread(JRealThreadPool * pool);
/* 线程代理 */
static void * j_thread_pool_thread_proxy(void * data);
/* 线程等待新任务，如果该函数返回NULL，则线程会进入公共线程池 */
static void * j_thread_pool_wait_for_new_task(JRealThreadPool * pool);
/* 线程现在空闲，为它寻找一个新的线程池 */
static JRealThreadPool *j_thread_pool_wait_for_new_pool(void);

/* 唤醒线程池中其他所有的线程，并结束线程池 */
static void j_thread_pool_wakeup_and_stop_all(JRealThreadPool * pool);

/* 往线程池里添加新任务 */
static void j_thread_pool_queue_push_unlocked(JRealThreadPool * pool,
        void * data);

/*
 * 创建一个线程池
 * @max_threads: 最大的线程数量，-1表示没有限制
 * @exclusive: 线程池是否与其他线程池共享线程
 */
JThreadPool *j_thread_pool_new(JFunc func, void * user_data,
                               int max_threads, boolean exclusive) {
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
        j_real_thread_pool_lock(pool);
        while (pool->num_threads < pool->max_threads) {
            if (!j_thread_pool_start_thread(pool)) {
                break;
            }
        }
        j_real_thread_pool_unlock(pool);
    }
    return (JThreadPool *) pool;
}

/*
 * 释放线程池
 * immediate：线程池是否立即终止，还是等待任务都完成
 * waiting：调用者是否等待线程池结束
 */
void j_thread_pool_free(JThreadPool * pool, boolean immediate,
                        boolean waiting) {
    JRealThreadPool *real = (JRealThreadPool *) pool;
    j_return_if_fail(real->running);

    /* 如果线程池中不允许有线程，并且任务不为空，那么应该立即结束 */
    j_return_if_fail(immediate || real->max_threads != 0 ||
                     j_async_queue_length(real->queue) == 0);

    j_real_thread_pool_lock(real);
    real->running = FALSE;
    real->immediate = immediate;
    real->waiting = waiting;

    if (waiting) {
        while (j_async_queue_length_unlocked(real->queue) !=
                -real->num_threads && !(immediate
                                        && real->num_threads == 0)) {
            /* 等待线程完成任务 */
            j_cond_wait(&real->cond, j_async_queue_get_mutex(real->queue));
        }
    }

    if (immediate
            || j_async_queue_length_unlocked(real->queue) ==
            -real->num_threads) {
        if (real->num_threads == 0) {
            /* 如果已经没有线程在执行了，做最后的清理 */
            j_real_thread_pool_unlock(real);
            j_thread_pool_free_internal(real);
            return;
        }
        j_thread_pool_wakeup_and_stop_all(real);
    }
    /* 最后一个线程执行线程池的清理 */
    real->waiting = FALSE;
    j_real_thread_pool_unlock(real);
}

/*
 * 将任务加入到线程池中
 */
boolean j_thread_pool_push(JThreadPool * pool, void * data) {
    JRealThreadPool *real = (JRealThreadPool *) pool;

    j_return_val_if_fail(real->running, FALSE);

    boolean result = TRUE;

    j_real_thread_pool_lock(real);
    if (j_async_queue_length_unlocked(real->queue) >= 0) {
        /* 当前没有线程在等待，则“启动“一个新线程 */
        if (!j_thread_pool_start_thread(real)) {
            result = FALSE;
        }
    }

    j_thread_pool_queue_push_unlocked(real, data);
    j_real_thread_pool_unlock(real);

    return result;
}

/* 往线程池里添加新任务 */
static void j_thread_pool_queue_push_unlocked(JRealThreadPool * pool,
        void * data) {
    if (pool->sort_func) {
        j_async_queue_push_sorted_unlocked(pool->queue, data,
                                           pool->sort_func,
                                           pool->sort_user_data);
    } else {
        j_async_queue_push_unlocked(pool->queue, data);
    }
}


static boolean j_thread_pool_start_thread(JRealThreadPool * pool) {
    boolean success = FALSE;

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
            j_thread_try_new("pool", j_thread_pool_thread_proxy, pool);
        if (thread == NULL) {
            return FALSE;
        }
        j_thread_unref(thread);
    }

    pool->num_threads++;
    return TRUE;
}

static void * j_thread_pool_thread_proxy(void * data) {
    JRealThreadPool *pool = (JRealThreadPool *) data;

    j_real_thread_pool_lock(pool);

    while (TRUE) {
        void * task = j_thread_pool_wait_for_new_task(pool);
        if (task) {
            if (pool->running || !pool->immediate) {
                /* 收到一个任务，并且线程池还是活跃的，执行任务 */
                j_real_thread_pool_unlock(pool);
                pool->pool.func(task, pool->pool.user_data);
                j_real_thread_pool_lock(pool);
                continue;
            }
        }
        /* "善后"工作 */
        boolean free_pool = FALSE;
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
        j_real_thread_pool_unlock(pool);
        if (free_pool) {
            j_thread_pool_free_internal(pool);
        }
        if ((pool = j_thread_pool_wait_for_new_pool()) == NULL) {
            break;
        }

        j_real_thread_pool_lock(pool);
    }
    return NULL;
}

/* 线程等待新任务，如果该函数返回NULL，则线程会进入公共线程池 */
static void * j_thread_pool_wait_for_new_task(JRealThreadPool * pool) {
    void * task = NULL;
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

static void j_thread_pool_wakeup_and_stop_all(JRealThreadPool * pool) {
    j_return_if_fail(pool->running == FALSE);
    j_return_if_fail(pool->num_threads != 0);
    pool->immediate = TRUE;

    int i;
    /* 给线程发送1后，线程会第一次会发现线程已经不再活跃，不会执行任何任务
     * 这时它会进入"善后"工作
     */
    for (i = 0; i < pool->num_threads; i++) {
        j_async_queue_push_unlocked(pool->queue, JUINT_TO_JPOINTER(1));
    }
}

static void j_thread_pool_free_internal(JRealThreadPool * pool) {
    j_return_if_fail(pool->running == FALSE);
    j_return_if_fail(pool->num_threads == 0);

    j_async_queue_unref(pool->queue);
    j_cond_clear(&pool->cond);

    j_free(pool);
}

/* 线程现在空闲，为它寻找一个新的线程池 */
static JRealThreadPool *j_thread_pool_wait_for_new_pool(void) {
    int local_wakeup_thread_serial;
    unsigned int local_max_unused_threads;
    int local_max_idle_time;
    int last_wakeup_thread_serial;
    boolean have_relayed_thread_marker = FALSE;

    local_max_unused_threads = j_atomic_int_get(&max_unused_threads);
    local_max_idle_time = j_atomic_int_get(&max_idle_time);
    last_wakeup_thread_serial = j_atomic_int_get(&wakeup_thread_serial);

    j_atomic_int_inc(&unused_threads);

    JRealThreadPool *pool = NULL;
    do {
        if (j_atomic_int_get(&unused_threads) >= local_max_unused_threads) {
            /* 未使用的线程数量过多 */
            pool = NULL;
        } else if (local_max_idle_time > 0) {
            /* 如果最大等待时间设置了，那么等待该时间 */
            pool = j_async_queue_timeout_pop(unused_thread_queue,
                                             local_max_idle_time * 1000);
        } else {                /* 否则永久等待 */
            pool = j_async_queue_pop(unused_thread_queue);
        }

        if (pool == wakeup_thread_marker) {
            local_wakeup_thread_serial =
                j_atomic_int_get(&wakeup_thread_serial);
            if (local_wakeup_thread_serial == last_wakeup_thread_serial) {
                if (!have_relayed_thread_marker) {
                    /* 如果该唤醒指针第二次收到了，则重新放入队列 */
                    j_async_queue_push(unused_thread_queue,
                                       (void*)wakeup_thread_marker);
                    have_relayed_thread_marker = TRUE;
                    j_usleep(100);
                }
            }
        } else {
            if (j_atomic_int_add(&kill_unused_threads, -1) > 0) {
                pool = NULL;
                break;
            }
            local_max_unused_threads =
                j_atomic_int_get(&max_unused_threads);
            local_max_idle_time = j_atomic_int_get(&max_idle_time);
            last_wakeup_thread_serial = local_wakeup_thread_serial;

            have_relayed_thread_marker = FALSE;
        }
    } while (pool == wakeup_thread_marker);
    j_atomic_int_add(&unused_threads, -1);
    return pool;
}

/*
 * 获取线程池中线程等待下一个任务的最长等待时间
 * 如果是0则表示无限等待
 * 这是一个全局设置
 */
unsigned int j_thread_pool_get_max_idle_time(void) {
    return j_atomic_int_get(&max_idle_time);
}

void j_thread_pool_set_max_idle_time(unsigned int interval) {
    j_atomic_int_set(&max_idle_time, interval);

    unsigned int i = j_atomic_int_get(&unused_threads);
    if (i > 0) {
        /* 唤醒等待中的线程，让他们根据新的等待时间等待 */
        j_atomic_int_inc(&wakeup_thread_serial);
        j_async_queue_lock(unused_thread_queue);
        do {
            j_async_queue_push_unlocked(unused_thread_queue,
                                        (void*)wakeup_thread_marker);
        } while (--i);

        j_async_queue_unlock(unused_thread_queue);
    }
}

/* 获取最大的和当前正在执行的线程数量 */
int j_thread_pool_get_max_threads(JThreadPool * pool) {
    JRealThreadPool *real = (JRealThreadPool *) pool;
    j_return_val_if_fail(real->running, 0);

    j_real_thread_pool_lock(real);
    int max_threads = real->max_threads;
    j_real_thread_pool_unlock(real);

    return max_threads;
}

int j_thread_pool_get_num_threads(JThreadPool * pool) {
    JRealThreadPool *real = (JRealThreadPool *) pool;
    j_return_val_if_fail(real->running, 0);

    j_real_thread_pool_lock(real);
    int max_threads = real->num_threads;
    j_real_thread_pool_unlock(real);

    return max_threads;
}
