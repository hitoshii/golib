#include <jio/jio.h>

#define LISTEN_PORT 42123

static jboolean async_result = FALSE;

static jboolean accept_callback(JSocket * socket, JSocket * conn,
                                jpointer user_data)
{
    if (conn) {
        j_socket_unref(conn);
        async_result = TRUE;
    }
    j_socket_unref(socket);
    j_quit();
    return FALSE;
}

static jboolean timeout_callback(jpointer user_data)
{
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    JSocketAddress addr;
    if (socket == NULL
        || !j_inet_socket_address_init_from_string(&addr, "127.0.0.1",
                                                   LISTEN_PORT)) {
        return FALSE;
    };
    if (!j_socket_connect(socket, &addr)) {
        return FALSE;
    }
    j_socket_unref(socket);
    return FALSE;
}


int main(int argc, char const *argv[])
{
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    JSocketAddress addr;
    if (!j_inet_socket_address_init_from_string
        (&addr, "127.0.0.1", LISTEN_PORT)) {
        return 1;
    }
    if (!j_socket_bind(socket, &addr, TRUE)) {
        return 2;
    }
    if (!j_socket_listen(socket, 1024)) {
        return 3;
    }
    j_socket_accept_async(socket, accept_callback, NULL);
    j_timeout_add(1000, timeout_callback, NULL);
    j_main();
    if (!async_result) {
        return 4;
    }
    return 0;
}
