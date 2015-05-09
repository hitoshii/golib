#include <jlib/jlib.h>


void log_handler(const jchar * domain, JLogLevelFlag level,
                 const jchar * message, jpointer user_data)
{
}

int main(int argc, char *argv[])
{
    j_log_set_handler("hello", log_handler, "");
    j_log_remove_handler("hello");
    return 0;
}
