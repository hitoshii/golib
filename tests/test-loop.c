#include <jio/jio.h>
#include <stdio.h>
#include <stdlib.h>

static int result = 0;

static void send_callback(JSocket * sock, const void *data,
                          unsigned int count, unsigned int len,
                          void *user_data)
{
    if (count == len) {
        printf("send all: %u\n", len);
        result = 0;
    } else {
        printf("send %u/%u\n", len, count);
        result = 1;
    }
    j_socket_close(sock);
    j_main_quit();
}

static int async_callback(JSocket * listen, JSocket * conn,
                          void *user_data)
{
    if (conn == NULL) {
        printf("failed:%s\n", (char *) user_data);
        result = 1;
        j_main_quit();
    } else {
        printf("accepted:%s\n", (char *) user_data);
        j_socket_send_async(conn, send_callback, "hello world", 12, NULL);
    }

    return 0;
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

    j_socket_close(client);
    j_socket_close(jsock);
    return result;
}
