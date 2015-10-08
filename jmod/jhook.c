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


JList *get_client_accept_hooks(void) {
    return client_accept_hooks;
}

void register_client_accept(ClientAccept accept) {
    if(accept==NULL) {
        return;
    }
    client_accept_hooks = j_list_append(client_accept_hooks, accept);
}
