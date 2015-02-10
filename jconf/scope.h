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
#ifndef __J_CONF_SCOPE_H__
#define __J_CONF_SCOPE_H__

#include "node.h"

#define j_conf_scope_new(name)  j_conf_node_new(J_CONF_NODE_SCOPE,name)
#define j_conf_scope_free(s)    j_conf_node_free(s)

#define j_conf_scope_append_child(s, child) \
                        j_conf_node_append_child(s, child)


#endif
