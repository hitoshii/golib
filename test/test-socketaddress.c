#include <jio/jio.h>


int main(int argc, char const *argv[])
{
    JInetAddress *addr;
    addr = j_inet_address_new_any(J_SOCKET_FAMILY_INET);
    if (!j_inet_address_is_any(addr)) {
        return 1;
    }
    j_inet_address_free(addr);

    addr = j_inet_address_new_loopback(J_SOCKET_FAMILY_INET);
    if (!j_inet_address_is_loopback(addr)) {
        return 2;
    }
    j_inet_address_free(addr);

    addr = j_inet_address_new_from_string("115.28.32.21");
    jchar *string = j_inet_address_to_string(addr);
    if (j_strcmp0(string, "115.28.32.21")) {
        return 3;
    }
    j_free(string);
    j_inet_address_free(addr);
    return 0;
}
