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

#ifndef __JMOD_PROXY_H__
#define __JMOD_PROXY_H__

#include "jhook.h"


typedef void (*JacSendFunc)(JSocket *socket, const void *buf, unsigned int size, void *user_data);

void register_jac_send(JacSendFunc send);
void jac_send(JSocket *socket, const void *buf, unsigned int size, void *user_data);

typedef boolean (*JacSocketFilter)(JSocket *socket, void *data);
typedef void (*JacSendMultiFunc)(const void *buf, unsigned int size, void *user_data, JacSocketFilter filter, void *filter_data);

void register_jac_send_multi(JacSendMultiFunc send_multi);
void jac_send_multi(const void *buf, unsigned int size, void *user_data, JacSocketFilter filter, void *filter_data);


#endif
