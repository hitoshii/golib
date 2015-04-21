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


/*****************************ACTION ******************************/

typedef enum {
    J_MODULE_ACTION_DROP = 0x1,
    J_MODULE_ACTION_SEND = 0x2,
    J_MODULE_ACTION_RECV = 0x4,
    J_MODULE_ACTION_IGNORE = 0x8,
} JModuleActionType;

typedef struct {
    unsigned int type;
    JByteArray *array;
} JModuleAction;
#define j_module_action_get_type(act)   ((act)->type)
#define j_module_action_set_type(act, t)   (act)->type=(t)
#define j_module_action_get_byte_array(act) ((act)->array)
#define j_module_action_get_data(act)   (j_byte_array_get_data(j_module_action_get_byte_array(act)))
#define j_module_action_get_len(act)    (j_byte_array_get_len(j_module_action_get_byte_array(act)))

JModuleAction *j_module_action_new(void);
JModuleAction *j_module_action_new_with_type(unsigned int type);
void j_module_action_free(JModuleAction * act);

/**************************** ACCEPT ******************************/

typedef void (*JModuleAcceptHook) (JSocket * conn, JModuleAction * act);
typedef void (*JModuleAcceptErrorHook) (void);

/************************* RECV **********************************/

typedef void (*JModuleRecvHook) (JSocket * conn, const void *data,
                                 unsigned int len, JModuleAction * act);

typedef void (*JModuleRecvErrorHook) (JSocket * conn, const void *data,
                                      unsigned int len);


/*********************** SEND ***********************************/

typedef void (*JModuleSendHook) (JSocket * conn, const char *data,
                                 unsigned int count, JModuleAction * act);

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
