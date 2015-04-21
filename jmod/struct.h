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

/*
 * 模块初始化函数
 * r和n是模块相关的配置节点，
 * r在根配置下
 * n在VirtualServer配置下
 * 如果模块是在全局载入的，那么n一定为NULL，而r根据配置是否存在决定
 * 如果模块在VirtualServer中载入，那么r和n都根据配置是否存在而决定
 */
typedef void (*JModuleInit) (JConfNode * r, JConfNode * n);

typedef struct {
    char *name;                 /* module name */

    JModuleInit init;           /* 当模块载入时执行 */
} JModule;


#endif
