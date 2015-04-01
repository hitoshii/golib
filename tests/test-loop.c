#include <jio/jio.h>
#include <stdio.h>
#include <stdlib.h>

static int result = 0;
static int send = 0;


static void recv_callback(JSocket * sock, const char *data,
                          unsigned int count,
                          JSocketRecvResultType type, void *user_data)
{
    if (type == J_SOCKET_RECV_ERR || data == NULL || count == 0) {
        result = 1;
        printf("recv error! %d, %d\n", type, count);
    } else {
        if (count != 7) {
            result = 1;
            printf("recv fail!\n");
        } else {
            result = 0;
            char *buf = j_strndup(data, count);
            printf("%s!\n", buf);
            j_free(buf);
            result = 0;
        }
    }
    if (send == 1) {
        j_socket_close(sock);
        send = 2;
    } else {
        send = 1;
    }
}

static void send_callback(JSocket * sock, const void *data,
                          unsigned int count, unsigned int len,
                          void *user_data)
{
    if (count == len) {
        printf("send all: %u\n", len);
        result = 0;
        if (send == 1) {
            j_socket_close(sock);
            send = 2;
        } else {
            send = 1;
        }
    } else {
        printf("send %u/%u\n", len, count);
        result = 1;
        j_socket_close(sock);
        j_main_quit();
    }
}

static void async_callback(JSocket * listen, JSocket * conn,
                           void *user_data)
{
    if (conn == NULL) {
        printf("failed:%s\n", (char *) user_data);
        result = 1;
        j_main_quit();
    } else {
        printf("accepted:%s\n", (char *) user_data);
        j_socket_send_async(conn, send_callback, "hello world", 11, NULL);
        j_socket_send_async(conn, send_callback, "hello world", 5, NULL);
        j_socket_recv_len_async(conn, recv_callback, 7, user_data);
    }

    j_socket_close(listen);
}

static void client_recv_callback(JSocket * sock, const char *data,
                                 unsigned int count,
                                 JSocketRecvResultType type,
                                 void *user_data)
{
    if (count > 0) {
        char *buf = j_strndup(data, count);
        printf("client: %s!\n", buf);
        j_free(buf);
    } else {
        result = 1;
    }
    j_socket_close(sock);
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
    j_socket_send(client, "hello 111", 7);
    j_socket_recv_len_async(client, client_recv_callback, 13, NULL);

    j_main();

    return result;
}
