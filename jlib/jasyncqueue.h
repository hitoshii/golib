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
#ifndef __JLIB_ASYNC_QUEUE_H__
#define __JLIB_ASYNC_QUEUE_H__

#include "jobject.h"
#include "jthread.h"

typedef struct _JAsyncQueue JAsyncQueue;


JAsyncQueue *j_async_queue_new(void);
JAsyncQueue *j_async_queue_new_full(JDestroyNotify free_func);

#define j_async_queue_ref(queue) J_OBJECT_REF(queue)
#define j_async_queue_unref(queue) J_OBJECT_UNREF(queue)

void j_async_queue_lock(JAsyncQueue * queue);
void j_async_queue_unlock(JAsyncQueue * queue);

void j_async_queue_push(JAsyncQueue * queue, void * data);
void j_async_queue_push_unlocked(JAsyncQueue * queue, void * data);
void j_async_queue_push_sorted(JAsyncQueue * queue, void * data,
                               JCompareDataFunc func, void * user_data);
void j_async_queue_push_sorted_unlocked(JAsyncQueue * queue, void * data,
                                        JCompareDataFunc func,
                                        void * user_data);

void * j_async_queue_pop(JAsyncQueue * queue);
void * j_async_queue_pop_unlocked(JAsyncQueue * queue);
void * j_async_queue_try_pop(JAsyncQueue * queue);

void * j_async_queue_timeout_pop(JAsyncQueue * queue, uint64_t timeout);
void * j_async_queue_timeout_pop_unlocked(JAsyncQueue * queue,
        uint64_t timeout);

/*
 * 该长度是队列中实际数据数量减去等待该队列的线程数量
 * 因此它可能小与0
 * 当该长度小与0时，表示有线程阻塞在该队列
 * 当该长度大与0时，表示队列中有多余的数据，调用j_async_queue_pop*()不会阻塞
 */
int j_async_queue_length(JAsyncQueue * queue);
int j_async_queue_length_unlocked(JAsyncQueue * queue);

void j_async_queue_sort(JAsyncQueue * queue, JCompareDataFunc func,
                        void * user_data);
void j_async_queue_sort_unlocked(JAsyncQueue * queue,
                                 JCompareDataFunc func,
                                 void * user_data);

JMutex *j_async_queue_get_mutex(JAsyncQueue * queue);

#endif
