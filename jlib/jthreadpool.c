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


typedef struct {
    JThreadPool pool;
} JRealThreadPool;


/*
 * 创建一个线程池
 * @max_threads: 最大的线程数量，-1表示没有限制
 * @exclusive: 线程池是否与其他线程池共享线程
 */
JThreadPool *j_thread_pool_new(JFunc func, jpointer user_data,
                               jint max_threads, jboolean exlusive,
                               JError ** error)
{
    return NULL;
}
