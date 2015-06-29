#include <jio/jio.h>
#include <arpa/inet.h>


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
    saddr =
        j_inet_socket_address_new_from_string("fe80::665a:4ff:fea0:cbaa",
                                              23456);
    if (saddr == NULL) {
        return 6;
    }
    string = j_inet_socket_address_to_string(saddr);
    if (j_strcmp0(string, "fe80::665a:4ff:fea0:cbaa:23456")) {
        return 7;
    }
    j_free(string);

    struct sockaddr_in6 buf;
    if (!j_socket_address_to_native
        (saddr, &buf, j_socket_address_get_native_size(saddr), NULL)
        || buf.sin6_family != AF_INET6) {
        return 8;
    }
    char buffer[256];
    string =
        j_inet_address_to_string(j_inet_socket_address_get_address(saddr));
    if (inet_ntop(AF_INET6, &buf.sin6_addr.s6_addr, buffer, sizeof(buffer))
        == NULL || j_strcmp0(string, "fe80::665a:4ff:fea0:cbaa")) {
        return 9;
    }
    j_free(string);
    j_socket_address_free(saddr);
    return 0;
}
