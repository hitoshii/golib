/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */
#ifndef __JIO_ENUM_H__
#define __JIO_ENUM_H__

#include <sys/types.h>
#include <netinet/in.h>
#include <poll.h>

/* UNIX套接字的地址类型 */
typedef enum {
    J_UNIX_SOCKET_ADDRESS_INVALID,
    J_UNIX_SOCKET_ADDRESS_ANONYMOUS,
    J_UNIX_SOCKET_ADDRESS_PATH,
    J_UNIX_SOCKET_ADDRESS_ABSTRACT,
    J_UNIX_SOCKET_ADDRESS_ABSTRACT_PADDED,
} JUnixSocketAddressType;

/* 套接字协议族 */
typedef enum {
    J_SOCKET_FAMILY_INVALID,
    J_SOCKET_FAMILY_UNIX = AF_UNIX,
    J_SOCKET_FAMILY_INET = AF_INET,
    J_SOCKET_FAMILY_INET6 = AF_INET6
} JSocketFamily;

/* 套接字具体协议（区别协议族） */
typedef enum {
    J_SOCKET_PROTOCOL_INVALID = -1,
    J_SOCKET_PROTOCOL_DEFAULT = 0,  /* 使用默认的协议 */
    J_SOCKET_PROTOCOL_TCP = IPPROTO_TCP,
    J_SOCKET_PROTOCOL_UDP = IPPROTO_UDP,
    J_SOCKET_PROTOCOL_SCTP = IPPROTO_SCTP,
} JSocketProtocol;

/* 套接字类型，套接字类型并不与套接字的协议类型直接相关 */
typedef enum {
    J_SOCKET_TYPE_INVALID,
    J_SOCKET_TYPE_STREAM = SOCK_STREAM,
    J_SOCKET_TYPE_DATAGRAM = SOCK_DGRAM,
    /*
     * http://urchin.earth.li/~twic/Sequenced_Packets_Over_Ordinary_TCP.html
     * IP协议并不支持此类型的套接字，这里也不支持
     */
    J_SOCKET_TYPE_SEQPACKET = J_SOCKET_TYPE_INVALID,
} JSocketType;


typedef enum {
    J_POLL_IN = POLLIN,
    J_POLL_PRI = POLLPRI,
    J_POLL_OUT = POLLOUT,
    J_POLL_ERR = POLLERR,
    J_POLL_HUP = POLLHUP,
    J_POLL_NVAL = POLLNVAL
} JPollCondition;

#endif
