/*
 * Copyright (C) 2015  Wiky L
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "jsocketaddress.h"
#include <string.h>
#include <arpa/inet.h>

struct _JSocketAddress {
    JSocketFamily family;
};

struct _JSocketInetAddress {

};

struct _JSocketUnixAddress {

};

struct _JInetAddress {
    JSocketFamily family;
    union {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    } addr;
};

#define J_INET_ADDRESS_FAMILY_IS_VALID(family) ((family)==J_SOCKET_FAMILY_INET||(family)==J_SOCKET_FAMILY_INET6)
#define j_inet_address_is_inet(addr) ((addr)->family==J_SOCKET_FAMILY_INET)

JInetAddress *j_inet_address_new_any(JSocketFamily family)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    addr->family = family;
    if (family == J_SOCKET_FAMILY_INET) {
        addr->addr.ipv4.s_addr = htonl(INADDR_ANY);
    } else {
        memcpy(&addr->addr.ipv6, in6addr_any.s6_addr,
               sizeof(addr->addr.ipv6));
    }
    return addr;
}

JInetAddress *j_inet_address_new_loopback(JSocketFamily family)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    addr->family = family;
    if (family == J_SOCKET_FAMILY_INET) {
        addr->addr.ipv4.s_addr = htonl(INADDR_LOOPBACK);
    } else {
        memcpy(&addr->addr.ipv6, in6addr_loopback.s6_addr,
               sizeof(addr->addr.ipv6));
    }
    return addr;
}

JInetAddress *j_inet_address_new_from_bytes(JSocketFamily family,
                                            const juint8 * bytes)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    addr->family = family;
    if (j_inet_address_is_inet(addr)) {
        memcpy(&addr->addr, bytes, sizeof(addr->addr.ipv4));
    } else {
        memcpy(&addr->addr, bytes, sizeof(addr->addr.ipv6));
    }
    return addr;
}

JInetAddress *j_inet_address_new_from_string(const jchar * string)
{
    struct in_addr addr4;
    struct in6_addr addr6;

    if (inet_pton(AF_INET, string, &addr4) > 0) {
        return j_inet_address_new_from_bytes(J_SOCKET_FAMILY_INET,
                                             (juint8 *) & addr4);
    } else if (inet_pton(AF_INET6, string, &addr6) > 0) {
        return j_inet_address_new_from_bytes(J_SOCKET_FAMILY_INET6,
                                             (juint8 *) & addr6);
    }
    return NULL;
}

JSocketFamily j_inet_address_get_family(JInetAddress * addr)
{
    return addr->family;
}

const juint8 *j_inet_address_to_bytes(JInetAddress * addr)
{
    return (juint8 *) & addr->addr;
}

jchar *j_inet_address_to_string(JInetAddress * addr)
{
    jchar buffer[INET6_ADDRSTRLEN];
    if (j_inet_address_is_inet(addr)) {
        inet_ntop(AF_INET, &addr->addr.ipv4, buffer, sizeof(buffer));
    } else {
        inet_ntop(AF_INET6, &addr->addr.ipv6, buffer, sizeof(buffer));
    }
    return j_strdup(buffer);
}

juint j_inet_address_get_size(JInetAddress * addr)
{
    if (j_inet_address_is_inet(addr)) {
        return sizeof(addr->addr.ipv4);
    }
    return sizeof(addr->addr.ipv6);
}

jboolean j_inet_address_equal(JInetAddress * addr, JInetAddress * another)
{
    if (addr->family != another->family) {
        return FALSE;
    }
    return memcmp(j_inet_address_to_bytes(addr),
                  j_inet_address_to_bytes(another),
                  j_inet_address_get_size(addr)) == 0;
}

jboolean j_inet_address_is_any(JInetAddress * addr)
{
    if (j_inet_address_is_inet(addr)) {
        return ntohl(addr->addr.ipv4.s_addr) == INADDR_ANY;
    }
    return IN6_IS_ADDR_UNSPECIFIED(&addr->addr.ipv6);
}

jboolean j_inet_address_is_loopback(JInetAddress * addr)
{
    if (j_inet_address_is_inet(addr)) {
        juint32 addr4 = ntohl(addr->addr.ipv4.s_addr);
        /* 127.0.0.0/8 */
        return (addr4 & 0xff000000) == 0x7f000000;
    }
    return IN6_IS_ADDR_LOOPBACK(&addr->addr.ipv6);
}

jboolean j_inet_address_is_multicast(JInetAddress * addr)
{
    if (j_inet_address_is_inet(addr)) {
        juint32 addr4 = ntohl(addr->addr.ipv4.s_addr);
        return IN_MULTICAST(addr4);
    }
    return IN6_IS_ADDR_MULTICAST(&addr->addr.ipv6);
}

/*
 * 根据struct sockaddr结构创建JSocketAddress
 */
JSocketAddress *j_socket_address_from_native(jpointer native, juint size)
{
    jshort family = ((struct sockaddr *) native)->sa_family;

    if (family == AF_UNSPEC) {
        return NULL;
    } else if (family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *) native;
    } else if (family == AF_INET6) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *) native;
    }
    return NULL;
}

/* 获取地址结构的协议族 */
JSocketFamily j_socket_address_get_family(JSocketAddress * addr)
{
    return addr->family;
}
