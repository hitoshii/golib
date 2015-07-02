#include <jio/jio.h>

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
    j_socket_close(socket);
    return 0;
}
