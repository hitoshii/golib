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
#ifndef __JLIB_ASYNC_QUEUE_H__
#define __JLIB_ASYNC_QUEUE_H__

#include "jthread.h"

typedef struct _JAsyncQueue JAsyncQueue;


JAsyncQueue *j_async_queue_new(void);
JAsyncQueue *j_async_queue_new_full(JDestroyNotify free_func);

void j_async_queue_ref(JAsyncQueue * queue);
void j_async_queue_unref(JAsyncQueue * queue);

void j_async_queue_lock(JAsyncQueue * queue);
void j_async_queue_unlock(JAsyncQueue * queue);

void j_async_queue_push(JAsyncQueue * queue, jpointer data);
void j_async_queue_push_unlocked(JAsyncQueue * queue, jpointer data);
void j_async_queue_push_sorted(JAsyncQueue * queue, jpointer data,
                               JCompareDataFunc func, jpointer user_data);
void j_async_queue_push_sorted_unlocked(JAsyncQueue * queue, jpointer data,
                                        JCompareDataFunc func,
                                        jpointer user_data);

jpointer j_async_queue_pop(JAsyncQueue * queue);
jpointer j_async_queue_pop_unlocked(JAsyncQueue * queue);
jpointer j_async_queue_try_pop(JAsyncQueue * queue);

#endif
