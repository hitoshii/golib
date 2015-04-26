#include <jlib/jlib.h>
#include <stdio.h>

J_PRIVATE_DEFINE_STATIC(private, NULL);

static jpointer thread_func1(jpointer data)
{
    printf("%s\n", (const jchar *) data);
    return "hello";
}

static jpointer thread_func(jpointer data)
{
    JThread *thread = j_thread_new("test-thread", thread_func1,
                                   "My name is Han Meimei");
    jpointer retval = j_thread_join(thread);
    if (j_strcmp0(retval, "hello")) {
        return NULL;
    }
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
