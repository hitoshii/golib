#include <jio/jio.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
    JLogger *logger = j_logger_open("./log.test", "[%0] [%l]:::%m!");
    if (logger == NULL) {
        return 1;
    }
    j_logger_log(logger, J_LOG_LEVEL_INFO, "what's ok");
    j_logger_warning(logger, "what the fuck");
    j_logger_error(logger, "easy, just an error");
    j_logger_close(logger);
    return 0;
}
