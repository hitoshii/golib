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
#include "jhook.h"
#include <jlib/jlib.h>

static JList *client_accept_hooks = NULL;
static JList *client_recv_hooks = NULL;
static JList *client_send_hooks = NULL;
static JList *server_init_hooks = NULL;


JList *get_server_init_hooks(void) {
    return server_init_hooks;
}


JList *get_client_accept_hooks(void) {
    return client_accept_hooks;
}

JList *get_client_recv_hooks(void) {
    return client_recv_hooks;
}

JList *get_client_send_hooks(void) {
    return client_send_hooks;
}

void register_client_accept(AcceptClient accept) {
    if(accept==NULL) {
        return;
    }
    client_accept_hooks = j_list_append(client_accept_hooks, accept);
}

void register_client_recv(RecvClient recv) {
    if(recv==NULL) {
        return;
    }
    client_recv_hooks=j_list_append(client_recv_hooks, recv);
}

void register_client_send(SendClient send) {
    if(send==NULL) {
        return;
    }
    client_send_hooks=j_list_append(client_send_hooks, send);
}

void register_server_init(ServerInit init) {
    if(init==NULL) {
        return;
    }
    server_init_hooks = j_list_append(server_init_hooks, init);
}
