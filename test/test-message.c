#include <jlib/jlib.h>
#include <stdio.h>

static void log_handler(const jchar * domain, JLogLevelFlag level,
                        const jchar * message, jpointer user_data)
{
    printf("[%s] %s\n", domain, message);
}

int main(int argc, char *argv[])
{
    j_log(NULL, J_LOG_LEVEL_CRITICAL, "Critical!!!!");
    j_debug("这是一个DEBUG，%s", "是吗？");
    j_log_set_handler("test", log_handler, NULL);
    j_log("test", J_LOG_LEVEL_INFO, "Info!!!!");
    //j_error("ERROR");
    return 0;
}
