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
#include <jio/jio.h>

#define LISTEN_PORT 42123

static boolean async_result = FALSE;

static boolean accept_callback(JSocket * socket, JSocket * conn,
                               void * user_data) {
    if (conn) {
        j_socket_unref(conn);
        async_result = TRUE;
    }
    j_socket_unref(socket);
    j_main_quit();
    return FALSE;
}

static boolean timeout_callback(void * user_data) {
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    JSocketAddress addr;
    if (socket == NULL
            || !j_inet_socket_address_init_from_string(&addr, "127.0.0.1",
                    LISTEN_PORT)) {
        return FALSE;
    };
    if (!j_socket_connect(socket, &addr)) {
        return FALSE;
    }
    j_socket_unref(socket);
    return FALSE;
}


int main(int argc, char const *argv[]) {
    JSocket *socket =
        j_socket_new(J_SOCKET_FAMILY_INET, J_SOCKET_TYPE_STREAM,
                     J_SOCKET_PROTOCOL_TCP);
    JSocketAddress addr;
    if (!j_inet_socket_address_init_from_string
            (&addr, "127.0.0.1", LISTEN_PORT)) {
        return 1;
    }
    if (!j_socket_bind(socket, &addr, TRUE)) {
        return 2;
    }
    if (!j_socket_listen(socket, 1024)) {
        return 3;
    }
    j_socket_accept_async(socket, accept_callback, NULL);
    j_timeout_add(1000, timeout_callback, NULL);
    j_main();
    if (!async_result) {
        return 4;
    }
    return 0;
}
