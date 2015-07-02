#include <jio/jio.h>

int main(int argc, char const *argv[])
{
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    if (socket == NULL) {
        return 1;
    }
    j_socket_close(socket);
    return 0;
}
