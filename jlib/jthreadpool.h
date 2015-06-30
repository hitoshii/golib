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
#ifndef __JLIB_THREAD_POOL_H__
#define __JLIB_THREAD_POOL_H__

#include "jtypes.h"
#include "jerror.h"


typedef struct {
    JFunc func;
    jpointer user_data;
    jboolean exclusive;
} JThreadPool;


/*
 * 创建一个线程池
 * @max_threads: 最大的线程数量，-1表示没有限制
 * @exclusive: 线程池是否与其他线程池共享线程
 */
JThreadPool *j_thread_pool_new(JFunc func, jpointer user_data,
                               jint max_threads, jboolean exclusive);

/*
 * 释放线程池
 * immediate：线程池是否立即终止，还是等待任务都完成
 * waiting：调用者是否等待线程池结束
 */
void j_thread_pool_free(JThreadPool * pool, jboolean immediate,
                        jboolean waiting);

/*
 * 将任务加入到线程池中
 */
jboolean j_thread_pool_push(JThreadPool * pool, jpointer data);

/* 获取最大的和当前正在执行的线程数量 */
jint j_thread_pool_get_max_threads(JThreadPool * pool);
jint j_thread_pool_get_num_threads(JThreadPool * pool);

/*
 * 获取和设置线程池中线程等待下一个任务的最长等待时间，毫秒计
 * 如果是0则表示无限等待
 * 这是一个全局设置
 */
juint j_thread_pool_get_max_idle_time(void);
void j_thread_pool_set_max_idle_time(juint interval);

#endif
