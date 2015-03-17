#include <jio/jio.h>
#include <stdlib.h>



int main(int argc, char const *argv[])
{
    JPoll *poll = j_poll_new();
    if (poll == NULL) {
        return 1;
    }
    JSocket *listensock = j_socket_listen_on(23456, 128);
    if (listensock == NULL) {
        return 1;
    }
    if (!j_poll_ctl
        (poll, J_POLL_CTL_ADD, J_POLLIN, listensock, listensock)) {
        return 1;
    }

    JSocket *client = j_socket_connect_to("127.0.0.1", "23456");

    JPollEvent events[16];
    int n = j_poll_wait(poll, events, 16, -1);
    if (n != 1) {
        return 1;
    }
    JSocket *conn = j_socket_accept(listensock);
    if (conn == NULL) {
        return 1;
    }
    j_socket_close(conn);
    j_poll_close(poll);
    j_socket_close(client);
    j_socket_close(listensock);
    return 0;
}
