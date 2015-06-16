#include <jlib/jlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


jpointer worker_func(jpointer data)
{
    JAsyncQueue *queue = (JAsyncQueue *) data;
    j_async_queue_ref(queue);
    jint i = 0;
    juint usec = 10 * 1000;
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

jpointer consumer_func(jpointer d)
{
    JAsyncQueue *queue = (JAsyncQueue *) d;
    j_async_queue_ref(queue);

    jchar *data = NULL;
    do {
        j_free(data);
        data = (jchar *) j_async_queue_pop(queue);
        j_printf("consumer: %s\n", data);
    } while (j_strcmp0(data, "end"));
    j_free(data);

    j_async_queue_push(queue, j_strdup("end"));
    j_async_queue_unref(queue);
    return NULL;
}

int main(int argc, char *argv[])
{
    JAsyncQueue *queue = j_async_queue_new_full(j_free);
    j_thread_unref(j_thread_new("worker1", worker_func, queue));
    JThread *consumer = j_thread_new("consumer1", consumer_func, queue);
    jchar *data = NULL;
    do {
        j_free(data);
        data = (jchar *) j_async_queue_pop(queue);
        j_printf("main: %s\n", data);
    } while (j_strcmp0(data, "end"));
    j_free(data);
    j_thread_join(consumer);
    j_thread_unref(consumer);
    j_async_queue_unref(queue);
    return 0;
}
