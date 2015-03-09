#include <jio/jio.h>
#include <stdio.h>
#include <stdlib.h>

static int result = 0;

static void async_callback(JSocket * listen, JSocket * conn,
                           void *user_data)
{
    if (conn == NULL) {
        printf("failed:%s\n", (char *) user_data);
        result = 1;
    } else {
        printf("accepted:%s\n", (char *) user_data);
        result = 0;
    }
    j_main_quit();
}

int main(int argc, char *argv[])
{
    JSocket *jsock = j_socket_listen_on(22346, 32);
    if (jsock == NULL) {
        return 1;
    }
    j_socket_accept_async(jsock, async_callback, "nice");
    JSocket *client = j_socket_connect_to("127.0.0.1", "22346");

    j_main();
    return result;
}
