#include <jlib/jlib.h>
#include <jlib/jwakeup.h>
#include <stdio.h>
#include <unistd.h>

J_PRIVATE_DEFINE_STATIC(private, NULL);

static jpointer thread_func1(jpointer data)
{
    JWakeup *wakeup = (JWakeup *) data;
    sleep(1);
    j_wakeup_signal(wakeup);
    return "hello";
}

static jpointer thread_func(jpointer data)
{
    JWakeup *wakeup = j_wakeup_new();
    JThread *thread = j_thread_new("test-thread", thread_func1,
                                   wakeup);
    JEPoll *ep = j_epoll_new();
    j_epoll_ctl(ep, j_wakeup_get_pollfd(wakeup), J_EPOLL_CTL_ADD,
                J_EPOLL_IN, NULL, NULL);
    JEPollEvent event[1];
    jint n = j_epoll_wait(ep, event, 1, -1);
    jpointer retval = j_thread_join(thread);
    if (n != 1 || j_strcmp0((const jchar *) retval, "hello")) {
        return NULL;
    }
    j_thread_unref(thread);
    j_epoll_close(ep);
    j_wakeup_free(wakeup);
    printf("%s\n", (const jchar *) data);
    return "hello world";
}

int main(int argc, char *argv[])
{
    J_MUTEX_DEFINE(lock);

    j_mutex_lock(&lock);
    j_mutex_unlock(&lock);

    j_mutex_clear(&lock);

    jpointer data = j_private_get(&private);
    if (data != NULL) {
        return 1;
    }
    j_private_set(&private, (jpointer) 1);
    data = j_private_get(&private);
    if (data != (jpointer) 1) {
        return 1;
    }

    JThread *thread = j_thread_new("test-thread", thread_func,
                                   "My name is Jim Green");
    jpointer retval = j_thread_join(thread);
    if (j_strcmp0(retval, "hello world")) {
        return 1;
    }
    return 0;
}
