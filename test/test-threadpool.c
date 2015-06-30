#include <jlib/jlib.h>


void thread_func(jpointer data, jpointer user_data)
{
    JAsyncQueue *queue = (JAsyncQueue *) user_data;
    juint time = JPOINTER_TO_JUINT(data);

    j_async_queue_ref(queue);
    j_usleep(time * 200000);
    j_async_queue_push(queue, j_thread_self());
    j_async_queue_unref(queue);
}


int main(int argc, char *argv[])
{
    JPtrArray *ptrs = j_ptr_array_new();
    JAsyncQueue *queue = j_async_queue_new();
    JThreadPool *p1 = j_thread_pool_new(thread_func, queue, 3, TRUE);
    JThreadPool *p2 = j_thread_pool_new(thread_func, queue, 2, TRUE);
    JThreadPool *p3 = j_thread_pool_new(thread_func, queue, 1, TRUE);
    if (p1 == NULL || p2 == NULL) {
        return -1;
    }
    jint i;
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
        jpointer ptr = j_async_queue_pop(queue);
        j_ptr_array_append_unique_ptr(ptrs, ptr);
    }
    if (j_ptr_array_get_len(ptrs) != 6) {
        return -3;
    }
    j_ptr_array_free(ptrs, FALSE);
    j_async_queue_unref(queue);
    return 0;
}
