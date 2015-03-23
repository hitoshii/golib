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
#ifndef __J_MOD_HOOK_H__
#define __J_MOD_HOOK_H__

#include <jio/jio.h>
#include <jlib/jlib.h>


typedef enum {
    J_HOOK_ACCEPT,
    J_HOOK_RECV,
    J_HOOK_SEND,
} JModuleHookType;


/**************************** ACCEPT ******************************/
typedef enum {
    J_MODULE_ACCEPT_DROP,
    J_MODULE_ACCEPT_SEND,
    J_MODULE_ACCEPT_RECV,
} JModuleAcceptAct;

typedef struct {
    JByteArray *array;
    JModuleAcceptAct act;
} JModuleAccept;
#define j_module_accept_get_data(acc)   \
                j_byte_array_get_data((acc)->array)
#define j_module_accept_get_len(acc)    \
                j_byte_array_get_len((acc)->array)
#define j_module_accept_get_action(acc) \
                ((acc)->act)
#define j_module_accept_is_drop(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_DROP
#define j_module_accept_is_send(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_SEND
#define j_module_accept_is_recv(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_RECV

typedef void (*JModuleAcceptHook) (JSocket * conn, JModuleAccept * acc);

JModuleAccept *j_module_accept_new(void);
void j_module_accept_free(JModuleAccept * acc);

/*************************END of ACCEPT *************************/


#define J_RECV_IGNORE   0
#define J_RECV_DROP     1
#define J_RECV_RESPONSE 2
typedef int (*JModuleRecvHook) (JSocket * conn, JByteArray * req,
                                JByteArray * rep);

#define J_SEND_OK   0
#define J_SEND_DROP 1
typedef int (*JModuleSendHook) (JSocket * conn, const char *data,
                                unsigned int count, unsigned int len);

/*
 * 注册回调函数
 */
void j_mod_register_hook(JModuleHookType type, void *hook);

/*
 * 获取相应的回调函数列表
 */
JList *j_mod_get_hooks(JModuleHookType type);


#endif
