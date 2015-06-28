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
#include <netdb.h>

JSocketAddress *j_inet_socket_address_new(JInetAddress * iaddr,
                                          juint16 port)
{
    JSocketAddress *addr = j_malloc(sizeof(JSocketAddress));
    addr->family = j_inet_address_get_family(iaddr);
    memcpy(&addr->addr.inet.address, iaddr,
           sizeof(addr->addr.inet.address));
    addr->addr.inet.port = htons(port);
    return addr;
}

JSocketAddress *j_socket_address_new_from_native(jpointer native,
                                                 juint size)
{
    return NULL;
}

/* 创建一个网络地址  */
JSocketAddress *j_inet_socket_address_new_from_string(const jchar *
                                                      address, juint port)
{
    JSocketAddress *addr = NULL;
    if (strchr(address, ':')) {
        /* 可能是IPv6地址 */
        static struct addrinfo *hints, hints_struct;
        struct addrinfo *res;
        if (J_UNLIKELY(j_once_init_enter(&hints))) {
            hints_struct.ai_family = AF_UNSPEC;
            hints_struct.ai_socktype = SOCK_STREAM,
                hints_struct.ai_protocol = 0;
            hints_struct.ai_flags = AI_NUMERICHOST;
            j_once_init_leave(&hints, &hints_struct);
        }
        jint status = getaddrinfo(address, NULL, hints, &res);
        if (status != 0) {
            return NULL;
        }
        if (res->ai_family == AF_INET6
            && res->ai_addrlen == sizeof(struct sockaddr_in6)) {
            ((struct sockaddr_in6 *) res->ai_addr)->sin6_port =
                htons(port);
            addr =
                j_socket_address_new_from_native(res->ai_addr,
                                                 res->ai_addrlen);
        }
        freeaddrinfo(res);
    } else {
        /* 可能是IPv4地址 */
        JInetAddress iaddr;
        if (j_inet_address_init_from_string(&iaddr, address)) {
            addr = j_inet_socket_address_new(&iaddr, port);
        }
    }
    return addr;
}

#define J_INET_ADDRESS_FAMILY_IS_VALID(family) ((family)==J_SOCKET_FAMILY_INET||(family)==J_SOCKET_FAMILY_INET6)
#define j_inet_address_is_inet(addr) ((addr)->family==J_SOCKET_FAMILY_INET)

JInetAddress *j_inet_address_new_any(JSocketFamily family)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);

    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    j_inet_address_init_any(addr, family);
    return addr;
}

void j_inet_address_init_any(JInetAddress * addr, JSocketFamily family)
{
    j_return_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family));
    addr->family = family;
    if (family == J_SOCKET_FAMILY_INET) {
        addr->addr.ipv4.s_addr = htonl(INADDR_ANY);
    } else {
        memcpy(&addr->addr.ipv6, in6addr_any.s6_addr,
               sizeof(addr->addr.ipv6));
    }
}

JInetAddress *j_inet_address_new_loopback(JSocketFamily family)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    j_inet_address_init_loopback(addr, family);
    return addr;
}

void j_inet_address_init_loopback(JInetAddress * addr,
                                  JSocketFamily family)
{
    j_return_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family));
    addr->family = family;
    if (family == J_SOCKET_FAMILY_INET) {
        addr->addr.ipv4.s_addr = htonl(INADDR_LOOPBACK);
    } else {
        memcpy(&addr->addr.ipv6, in6addr_loopback.s6_addr,
               sizeof(addr->addr.ipv6));
    }
}

JInetAddress *j_inet_address_new_from_bytes(JSocketFamily family,
                                            const juint8 * bytes)
{
    j_return_val_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family), NULL);
    JInetAddress *addr = (JInetAddress *) j_malloc(sizeof(JInetAddress));
    j_inet_address_init_from_bytes(addr, family, bytes);
    return addr;
}

void j_inet_address_init_from_bytes(JInetAddress * addr,
                                    JSocketFamily family,
                                    const juint8 * bytes)
{
    j_return_if_fail(J_INET_ADDRESS_FAMILY_IS_VALID(family));
    addr->family = family;
    if (j_inet_address_is_inet(addr)) {
        memcpy(&addr->addr, bytes, sizeof(addr->addr.ipv4));
    } else {
        memcpy(&addr->addr, bytes, sizeof(addr->addr.ipv6));
    }
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

jboolean j_inet_address_init_from_string(JInetAddress * addr,
                                         const jchar * string)
{
    struct in_addr addr4;
    struct in6_addr addr6;

    if (inet_pton(AF_INET, string, &addr4) > 0) {
        j_inet_address_init_from_bytes(addr, J_SOCKET_FAMILY_INET,
                                       (juint8 *) & addr4);
    } else if (inet_pton(AF_INET6, string, &addr6) > 0) {
        j_inet_address_init_from_bytes(addr, J_SOCKET_FAMILY_INET6,
                                       (juint8 *) & addr6);
    } else {
        return FALSE;
    }
    return TRUE;
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
