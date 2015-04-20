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
#ifndef __JMOD_STRUCT_H__
#define __JMOD_STRUCT_H__


#include <jconf/jconf.h>
#include <jlib/jlib.h>

typedef void (*JModuleInit) (void); /* 模块的初始化函数 */


/* 服务进程开始时的回调函数 */
typedef void (*JModuleServerInit) (int pid, const char *name,
                                   unsigned short port);
typedef struct {
    char *name;                 /* module name */

    JModuleInit init;           /* 当模块载入时执行 */
    JModuleServerInit server_init;  /* 服务器开始时执行 */
} JModule;


#endif
