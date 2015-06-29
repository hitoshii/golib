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
#include <sys/un.h>

/* 套接字地址结构，所有结构都以网络字节序存储数据 */
typedef struct _JSocketAddress JSocketAddress;
typedef struct _JInetSocketAddress JInetSocketAddress;
typedef struct _JUnixSocketAddress JUnixSocketAddress;
typedef struct _JInetAddress JInetAddress;


/* 地址结构类型 */
struct _JInetAddress {
    JSocketFamily family;
    union {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    } addr;
};

struct _JInetSocketAddress {
    JInetAddress address;
    juint16 port;
    juint32 flowinfo;
    juint32 scope_id;
};

#define UNIX_PATH_MAX (sizeof(((struct sockaddr_un *)0)->sun_path) + 1)

/* XXX 还没有实现 */
struct _JUnixSocketAddress {
    jchar path[UNIX_PATH_MAX];
    JUnixSocketAddressType type;
};

struct _JSocketAddress {
    JSocketFamily family;
    union {
        JInetSocketAddress inet;    /* J_SOCKET_FAMILY_INET/J_SOCKET_FAMILY_INET6 */
        JUnixSocketAddress local;   /* J_SOCKET_FAMILY_UNIX 不能取变量名为unix，是一个默认的宏定义 */
    } addr;
};

#define j_socket_address_get_family(addr)   ((addr)->family)
#define j_socket_address_is_inet(addr)  (j_socket_address_get_family(addr)==J_SOCKET_FAMILY_INET)
#define j_socket_address_is_inet6(addr) (j_socket_address_get_family(addr)==J_SOCKET_FAMILY_INET6)
#define j_socket_address_is_unix(addr)  (j_socket_address_get_family(addr)==J_SOCKET_FAMILY_UNIX)
#define j_socket_address_free(addr) j_free(addr)

/*
 * 转化为一个struct sockaddr结构
 */
juint j_socket_address_get_native_size(JSocketAddress * addr);
jboolean j_socket_address_to_native(JSocketAddress * addr, jpointer dest,
                                    juint len, JError ** error);
/*
 * 从一个sockaddr结构创建
 */
JSocketAddress *j_socket_address_new_from_native(jpointer native,
                                                 juint size);
jboolean j_socket_address_init_from_native(JSocketAddress * saddr,
                                           jpointer native, juint size);
/* 创建一个网络地址  */
JSocketAddress *j_inet_socket_address_new(JInetAddress * addr,
                                          juint16 port);
void j_inet_socket_address_init(JSocketAddress * saddr,
                                JInetAddress * addr, juint16 port);
JSocketAddress *j_inet_socket_address_new_from_string(const jchar *
                                                      address, juint port);
jboolean j_inet_socket_address_init_from_string(JSocketAddress * saddr,
                                                const jchar * address,
                                                juint port);

/* 获取地址和端口号 */
jushort j_inet_socket_address_get_port(JSocketAddress * addr);
JInetAddress *j_inet_socket_address_get_address(JSocketAddress * addr);
/* 生成地址的字符串形式，如127.0.0.1:1234 */
jchar *j_inet_socket_address_to_string(JSocketAddress * addr);

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
jchar *j_inet_address_to_string_with_port(JInetAddress * addr,
                                          jushort port);
juint j_inet_address_get_native_size(JInetAddress * addr);
jboolean j_inet_address_is_any(JInetAddress * addr);    /* 该地址是否表示任意地址 */
jboolean j_inet_address_is_loopback(JInetAddress * addr);   /* 判断是否是回环地址，127.0.0.0/8 */
jboolean j_inet_address_is_multicast(JInetAddress * addr);  /* 判断是否是广播地址 */
#define j_inet_address_free(addr) j_free(addr);


#endif
