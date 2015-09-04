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
#include <jlib/jlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void * worker_func(void * data) {
    JAsyncQueue *queue = (JAsyncQueue *) data;
    j_async_queue_ref(queue);
    int i = 0;
    unsigned int usec = 10 * 1000;
    while (i <= 6) {
        usleep(usec * i);
        j_async_queue_lock(queue);
        j_async_queue_push_unlocked(queue, j_strdup_printf("msg%d", i));
        j_async_queue_push_unlocked(queue,
                                    j_strdup_printf("msg%d_secondary", i));
        j_async_queue_push_unlocked(queue,
                                    j_strdup_printf("msg%d_thirdary", i));
        j_async_queue_unlock(queue);
        i++;
    }
    j_async_queue_push(queue, j_strdup("end"));
    j_async_queue_push(queue, j_strdup("end"));
    j_async_queue_unref(queue);
    return NULL;
}

void * consumer_func(void * d) {
    JAsyncQueue *queue = (JAsyncQueue *) d;
    j_async_queue_ref(queue);

    char *data = NULL;
    do {
        j_free(data);
        data = (char *) j_async_queue_pop(queue);
        j_printf("consumer: %s\n", data);
    } while (j_strcmp0(data, "end"));
    j_free(data);

    j_async_queue_push(queue, j_strdup("end"));
    j_async_queue_unref(queue);
    return NULL;
}

int main(int argc, char *argv[]) {
    JAsyncQueue *queue = j_async_queue_new_full(j_free);
    j_thread_unref(j_thread_new("worker1", worker_func, queue));
    JThread *consumer = j_thread_new("consumer1", consumer_func, queue);
    char *data = NULL;
    do {
        j_free(data);
        data = (char *) j_async_queue_pop(queue);
        j_printf("main: %s\n", data);
    } while (j_strcmp0(data, "end"));
    j_free(data);
    j_thread_join(consumer);
    j_thread_unref(consumer);
    j_async_queue_unref(queue);
    return 0;
}
