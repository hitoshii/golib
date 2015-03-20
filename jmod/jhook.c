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
#include "jhook.h"
#include <jlib/jlib.h>

static JList *accept_hooks = NULL;
static JList *recv_hooks = NULL;
static JList *send_hooks = NULL;

/*
 * 获取相应的回调函数列表
 */
JList *j_mod_get_hooks(JModuleHookType type)
{
    switch (type) {
    case J_HOOK_ACCEPT:
        return accept_hooks;
    case J_HOOK_RECV:
        return recv_hooks;
    case J_HOOK_SEND:
        return send_hooks;
    }
    return NULL;
}

/*
 * 注册回调函数
 */
void j_mod_register_hook(JModuleHookType type, void *hook)
{
    switch (type) {
    case J_HOOK_ACCEPT:
        accept_hooks = j_list_append(accept_hooks, hook);
        break;
    case J_HOOK_RECV:
        recv_hooks = j_list_append(recv_hooks, hook);
        break;
    case J_HOOK_SEND:
        send_hooks = j_list_append(send_hooks, hook);
        break;
    }
}
