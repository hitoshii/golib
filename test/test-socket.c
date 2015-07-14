#include <jio/jio.h>

static jboolean async_result = FALSE;

static jboolean recv_callback(JSocket * socket, const jchar * buffer,
                              jint size, jpointer user_daata)
{
    if (size == 0) {
        async_result = TRUE;
        j_main_quit();
    } else if (size < 0) {
        j_main_quit();
    } else {
        j_printf("%.*s", size, buffer);
        return TRUE;
    }
    return FALSE;
}

static void send_callback(JSocket * socket, jint ret, jpointer user_data)
{
    if (ret <= 0) {
        j_main_quit();
        return;
    }
    j_socket_receive_async(socket, recv_callback, NULL);
    jchar buf[1024];
    jint n;
    while ((n = j_socket_receive(socket, buf, sizeof(buf) - 1)) > 0) {
        buf[n - 1] = '\0';
        printf("%s", buf);
    }
    async_result = TRUE;
    j_main_quit();
}

int main(int argc, char const *argv[])
{
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    if (socket == NULL) {
        return 1;
    }
    JSocketAddress addr;
    if (!j_inet_socket_address_init_from_string(&addr, "127.0.0.1", 23412)) {
        return 2;
    }
    if (!j_socket_bind(socket, &addr, TRUE)) {
        return 3;
    }
    if (!j_socket_listen(socket, 1024)) {
        return 4;
    }
    j_socket_unref(socket);

    socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    if (socket == NULL
        || !j_inet_socket_address_init_from_string(&addr, "115.29.105.159",
                                                   80)) {
        return 5;
    };

    j_socket_set_blocking(socket, FALSE);
    if (!j_socket_connect(socket, &addr)) {
        return 6;
    }

    jint err;
    while (!j_socket_check_connect_result(socket, &err) && err == 0) {
    }
    if (err) {
        return 7;
    }

    if (j_socket_send
        (socket,
         "GET / HTTP/1.1\r\nHost: test.scopehigh.top\r\nConnection: Close\r\n\r\n",
         -1) <= 0) {
        return 8;
    }
    jchar buf[1024];
    jint n;
    while ((n = j_socket_receive(socket, buf, sizeof(buf) - 1)) > 0) {
        buf[n - 1] = '\0';
    }
    if (n != 0) {
        return 9;
    }

    j_socket_close(socket);

    /* 测试异步读写数据 */
    socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    if (socket == NULL
        || !j_inet_socket_address_init_from_string(&addr, "192.30.252.128",
                                                   80)) {
        return 10;
    };
    if (!j_socket_connect(socket, &addr)) {
        return 11;
    }
    j_socket_send_async(socket,
                        "GET / HTTP/1.1\r\nHost: github.com\r\nConnection: Close\r\n\r\n",
                        -1, send_callback, NULL);

    j_socket_unref(socket);
    j_main();
    if (!async_result) {
        return 12;
    }
    return 0;
}
