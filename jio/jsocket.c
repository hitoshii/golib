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
#include "jsocket.h"
#include <sys/types.h>
#include <sys/socket.h>

/* j_socket_receive_from()的接收者缓存长度 */
#define RECV_ADDR_CACHE_SIZE 8

struct _JSocket {
    JSocketFamily family;
    JSocketType type;
    JSocketProtocol protocol;
    jint fd;

    jint listen_backlog;
    juint timeout;
    JSocketAddress *remote_address;

    juint inited:1;
    juint blocking:1;
    juint keepalive:1;
    juint closed:1;
    juint connected:1;
    juint listening:1;
    juint timed_out:1;
    juint connect_pending:1;

    struct {
        JSocketAddress *addr;
        struct sockaddr *native;
        jint native_len;
        juint64 last_used;
    } recv_addr_cache[RECV_ADDR_CACHE_SIZE];
};

JSocket *j_socket_new(JSocketFamily family, JSocketType type,
                      JSocketProtocol protocol)
{
    JSocket socket;
    return NULL;
}
