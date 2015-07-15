#include <jlib/jlib.h>
#include <stdio.h>


static jboolean idle(jpointer data)
{
    static jint i = 0;
    j_printf("idle - %d\n", i++);
    return TRUE;
}

static jboolean timeout(jpointer data)
{
    j_main_loop_quit((JMainLoop *) data);
    return FALSE;
}

int main(int argc, char *argv[])
{
    if (!j_daemonize()) {
        return 1;
    }
    JMainLoop *loop = j_main_loop_new(NULL, FALSE);
    j_timeout_add(1000, timeout, loop);
    j_idle_add(idle, NULL);
    j_main_loop_run(loop);
    j_main_loop_unref(loop);
    return 0;
}
