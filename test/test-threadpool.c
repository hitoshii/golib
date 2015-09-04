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

void thread_func(void * data, void * user_data) {
    JAsyncQueue *queue = (JAsyncQueue *) user_data;
    unsigned int time = JPOINTER_TO_JUINT(data);

    j_async_queue_ref(queue);
    j_usleep(time * 200000);
    j_async_queue_push(queue, j_thread_self());
    j_async_queue_unref(queue);
}


int main(int argc, char *argv[]) {
    JPtrArray *ptrs = j_ptr_array_new();
    JAsyncQueue *queue = j_async_queue_new();
    JThreadPool *p1 = j_thread_pool_new(thread_func, queue, 3, TRUE);
    JThreadPool *p2 = j_thread_pool_new(thread_func, queue, 2, TRUE);
    JThreadPool *p3 = j_thread_pool_new(thread_func, queue, 1, TRUE);
    if (p1 == NULL || p2 == NULL) {
        return -1;
    }
    int i;
    for (i = 1; i < 6; i++) {
        if (!j_thread_pool_push(p1, JUINT_TO_JPOINTER(i)) ||
                !j_thread_pool_push(p2, JUINT_TO_JPOINTER(7 - i)) ||
                !j_thread_pool_push(p3, JUINT_TO_JPOINTER(8 - i))) {
            return -2;
        }
    }

    j_thread_pool_free(p1, FALSE, TRUE);
    j_thread_pool_free(p2, FALSE, TRUE);
    j_thread_pool_free(p3, FALSE, TRUE);

    j_usleep(100000);

    i = 15;
    while (i--) {
        void * ptr = j_async_queue_pop(queue);
        j_ptr_array_append_ptr_unique(ptrs, ptr);
    }
    if (j_ptr_array_get_len(ptrs) != 6) {
        return -3;
    }
    j_ptr_array_free(ptrs, FALSE);
    j_async_queue_unref(queue);
    return 0;
}
