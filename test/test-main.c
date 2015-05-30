#include <jlib/jlib.h>
#include <stdio.h>


static jboolean timeout_seconds(jpointer data)
{
    j_main_loop_quit((JMainLoop *) data);
    return FALSE;
}

static jboolean timeout(jpointer data)
{
    j_timeout_add_seconds(1, timeout_seconds, data);
    return FALSE;
}

int main(int argc, char *argv[])
{
    JMainLoop *loop = j_main_loop_new(NULL, FALSE);
    j_timeout_add(1000, timeout, loop);
    j_main_loop_run(loop);
    j_main_loop_unref(loop);
    return 0;
}
