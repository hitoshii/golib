#include <jlib/jlib.h>
#include <stdio.h>

static jboolean timeout(jpointer data)
{
    static jint i = 0;
    j_printf("i=%d\n", i);
    i++;
    if (i >= 2) {
        j_main_loop_quit((JMainLoop *) data);
        return FALSE;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    JMainLoop *loop = j_main_loop_new(NULL, FALSE);
    j_timeout_add(1000, timeout, loop);
    j_main_loop_run(loop);
    j_main_loop_unref(loop);
    return 0;
}
