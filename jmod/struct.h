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

/* 在读取配置文件之前的初始化函数 */
typedef void (*JModuleConfigInit) (void);
/* 读取配置文件的回调函数，每个指令调用一次 */
typedef void (*JModuleLoadDirective) (const char *scope, JList * sargs,
                                      const char *directive,
                                      JList * dargs);
/* 配置文件读取结束后调用的函数 */
typedef void (*JModuleConfigSummary) (void);

/* 服务进程开始时的回调函数 */
typedef void (*JModuleServerInit) (int pid, const char *name,
                                   unsigned short port);

typedef struct {
    char **directives;          /* a NULL-terminated string array */
    char **scopes;              /* a NULL-terminated string array */

    JModuleConfigInit init;
    JModuleLoadDirective func;
    JModuleConfigSummary summary;
} JModuleConfigHandler;

typedef struct {
    char *name;                 /* module name */

    JModuleConfigHandler *config_handler;

    JModuleInit init;           /* 当模块载入时执行 */
    JModuleServerInit server_init;  /* 服务器开始时执行 */
} JModule;


#endif
