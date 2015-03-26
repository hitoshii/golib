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
    J_HOOK_ACCEPT_ERROR,
    J_HOOK_RECV,
    J_HOOK_RECV_ERROR,
    J_HOOK_SEND,
    J_HOOK_SEND_ERROR,
} JModuleHookType;


/**************************** ACCEPT ******************************/
typedef enum {
    J_MODULE_ACCEPT_DROP = 0,
    J_MODULE_ACCEPT_SEND,
    J_MODULE_ACCEPT_RECV,
    J_MODULE_ACCEPT_KEEP,       /* 不监听事件，单纯保持该连接 */
} JModuleAcceptAct;

typedef struct {
    JByteArray *array;
    JModuleAcceptAct act;
} JModuleAccept;
#define j_module_accept_get_byte_array(acc) \
                (acc)->array
#define j_module_accept_get_data(acc)   \
                j_byte_array_get_data(j_module_accept_get_byte_array(acc))
#define j_module_accept_get_len(acc)    \
                j_byte_array_get_len(j_module_accept_get_byte_array(acc))
#define j_module_accept_get_action(acc) \
                ((acc)->act)
#define j_module_accept_set_action(acc,action) (acc)->act=action
#define j_module_accept_is_drop(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_DROP
#define j_module_accept_is_send(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_SEND
#define j_module_accept_is_recv(acc)    \
                j_module_accept_get_action(acc)==J_MODULE_ACCEPT_RECV

typedef void (*JModuleAcceptHook) (JSocket * conn, JModuleAccept * acc);

JModuleAccept *j_module_accept_new(void);
void j_module_accept_free(JModuleAccept * acc);


typedef void (*JModuleAcceptErrorHook) (void);

/************************* END of ACCEPT *************************/

/************************* RECV **********************************/

typedef enum {
    J_MODULE_RECV_DROP = 0,
    J_MODULE_RECV_SEND,
    J_MODULE_RECV_RECV,
    J_MODULE_RECV_KEEP,         /* 不监听事件，单纯保持该连接 */
} JModuleRecvAct;

typedef struct {
    JByteArray *array;
    JModuleRecvAct act;
} JModuleRecv;
#define j_module_recv_get_byte_array(r) (r)->array
#define j_module_recv_get_data(r)   \
                j_byte_array_get_data(j_module_recv_get_byte_array(r))
#define j_module_recv_get_len(r)    \
                j_byte_array_get_len(j_module_recv_get_byte_array(r))
#define j_module_recv_get_action(r) (r)->act
#define j_module_recv_set_action(r,action) (r)->act=action
#define j_module_recv_is_recv(r)    \
                j_module_recv_get_action(r)==J_MODULE_RECV_RECV
#define j_module_recv_is_send(r)    \
                j_module_recv_get_action(r)==J_MODULE_RECV_SEND
#define j_module_recv_is_drop(r)    \
                j_module_recv_get_action(r)==J_MODULE_RECV_DROP

JModuleRecv *j_module_recv_new(void);
void j_module_recv_free(JModuleRecv * r);

typedef void (*JModuleRecvHook) (JSocket * conn, const void *data,
                                 unsigned int len,
                                 JSocketRecvResultType type,
                                 JModuleRecv * recv);

typedef void (*JModuleRecvErrorHook) (JSocket * conn, const void *data,
                                      unsigned int len,
                                      JSocketRecvResultType type);

/*********************** END of RECV *****************************/

/*********************** SEND ***********************************/
typedef enum {
    J_MODULE_SEND_DROP = 0,
    J_MODULE_SEND_SEND,
    J_MODULE_SEND_RECV,
    J_MODULE_SEND_KEEP,         /* 不监听事件，单纯保持该连接 */
} JModuleSendAct;

typedef struct {
    JByteArray *array;
    JModuleSendAct act;
} JModuleSend;

#define j_module_send_get_byte_array(s) (s)->array
#define j_module_send_get_data(s)   \
                j_byte_array_get_data(j_module_send_get_byte_array(s))
#define j_module_send_get_len(s)    \
                j_byte_array_get_len(j_module_send_get_byte_array(s))
#define j_module_send_get_action(s) (s)->act
#define j_module_send_set_action(s,action)  (s)->act=action
#define j_module_send_is_drop(s)    \
                j_module_send_get_action(s)==J_MODULE_SEND_DROP
#define j_module_send_is_send(s)    \
                j_module_send_get_action(s)==J_MODULE_SEND_SEND
#define j_module_send_is_recv(s)    \
                j_module_send_get_action(s)==J_MODULE_SEND_RECV

JModuleSend *j_module_send_new(void);
void j_module_send_free(JModuleSend * s);

typedef void (*JModuleSendHook) (JSocket * conn, const char *data,
                                 unsigned int count, unsigned int len,
                                 JModuleSend * send);

typedef void (*JModuleSendErrorHook) (JSocket * conn, const char *data,
                                      unsigned int count,
                                      unsigned int len);

/*********************** END of SEND ******************************/

/*
 * 注册回调函数
 */
void j_mod_register_hook(JModuleHookType type, void *hook);

/*
 * 获取相应的回调函数列表
 */
JList *j_mod_get_hooks(JModuleHookType type);


#endif
