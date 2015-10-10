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
#ifndef __JMOD_HOOK_H__
#define __JMOD_HOOK_H__

#include <jio/jio.h>
#include <jlib/jlib.h>


typedef void (*AcceptClient)(JSocket *socket);
typedef void (*RecvClient)(JSocket *socket, const char *buffer, int size, void *user_data);
typedef void (*SendClient)(JSocket *socket, int ret, void *user_data);


typedef struct {
    AcceptClient accept;
    RecvClient recv;
    SendClient send;
} JacHook;


JList *get_client_accept_hooks(void);
void register_client_accept(AcceptClient accept);
void register_client_recv(RecvClient recv);
void register_client_send(RecvClient send);


#endif
