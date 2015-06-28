#include <jio/jio.h>


int main(int argc, char const *argv[])
{
    JInetAddress *addr;
    addr = j_inet_address_new_any(J_SOCKET_FAMILY_INET);
    if (!j_inet_address_is_any(addr)) {
        return 1;
    }
    j_inet_address_free(addr);

    JInetAddress address;
    j_inet_address_init_loopback(&address, J_SOCKET_FAMILY_INET);
    if (!j_inet_address_is_loopback(&address)) {
        return 2;
    }
    j_inet_address_init_loopback(&address, J_SOCKET_FAMILY_INET6);
    if (!j_inet_address_is_loopback(&address)) {
        return 2;
    }

    if (!j_inet_address_init_from_string(&address, "115.28.32.123")) {
        return 3;
    }
    jchar *string = j_inet_address_to_string(&address);
    if (j_strcmp0(string, "115.28.32.123")) {
        return 4;
    }
    j_free(string);

    JSocketAddress *saddr =
        j_inet_socket_address_new_from_string("115.23.12.41", 12345);
    if (saddr == NULL) {
        return 5;
    }
    j_socket_address_free(saddr);
    return 0;
}
