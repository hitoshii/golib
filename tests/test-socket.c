#include <jio/jio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    JSocket *jsock = j_socket_listen_on(12346, 32);
    if (jsock == NULL) {
        return 1;
    }
    JSocket *client = j_socket_connect_to("127.0.0.1", "12346");
    JSocket *acc = j_socket_accept(jsock);
    if (client == NULL || acc == NULL) {
        return 1;
    }
    j_socket_close(jsock);
    j_socket_close(client);
    j_socket_close(acc);
    return 0;
}
