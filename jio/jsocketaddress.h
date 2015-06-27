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

#ifndef __JIO_SOCKETADDRESS_H__
#define __JIO_SOCKETADDRESS_H__

#include "jioenum.h"
#include <jlib/jlib.h>

/* 套接字地址结构 */
typedef struct _JSocketAddress JSocketAddress;
typedef struct _JSocketInetAddress JSocketInetAddress;
typedef struct _JSocketUnixAddress JSocketUnixAddress;
typedef struct _JInetAddress JInetAddress;


struct _JInetAddress {
    JSocketFamily family;
    union {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    } addr;
};

/*
 * JInetAddress IP地址结构，不包含端口号
 */
JInetAddress *j_inet_address_new_any(JSocketFamily family);
void j_inet_address_init_any(JInetAddress * addr, JSocketFamily family);
JInetAddress *j_inet_address_new_loopback(JSocketFamily family);
void j_inet_address_init_loopback(JInetAddress * addr,
                                  JSocketFamily family);
JInetAddress *j_inet_address_new_from_bytes(JSocketFamily family,
                                            const juint8 * bytes);
void j_inet_address_init_from_bytes(JInetAddress * addr,
                                    JSocketFamily family,
                                    const juint8 * bytes);
JInetAddress *j_inet_address_new_from_string(const jchar * string);
jboolean j_inet_address_init_from_string(JInetAddress * addr,
                                         const jchar * string);
JSocketFamily j_inet_address_get_family(JInetAddress * addr);
jboolean j_inet_address_equal(JInetAddress * addr, JInetAddress * another);
const juint8 *j_inet_address_to_bytes(JInetAddress * addr);
jchar *j_inet_address_to_string(JInetAddress * addr);
juint j_inet_address_get_size(JInetAddress * addr);
jboolean j_inet_address_is_any(JInetAddress * addr);    /* 该地址是否表示任意地址 */
jboolean j_inet_address_is_loopback(JInetAddress * addr);   /* 判断是否是回环地址，127.0.0.0/8 */
jboolean j_inet_address_is_multicast(JInetAddress * addr);  /* 判断是否是广播地址 */
#define j_inet_address_free(addr) j_free(addr);

/*
 * 根据struct sockaddr结构创建JSocketAddress
 */
JSocketAddress *j_socket_address_from_native(jpointer native, juint size);
/* 获取地址结构的协议族 */
JSocketFamily j_socket_address_get_family(JSocketAddress * addr);


#endif
