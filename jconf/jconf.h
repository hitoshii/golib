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
#ifndef __J_CONF_H__
#define __J_CONF_H__

#include "struct.h"

/*
 * 从一个文件中载入配置
 */
JConfRoot *j_conf_load_from_file(const char *path);


#define J_CONF_SUCCESS 0
#define J_CONF_ERROR_FILE   1   /* 打开文件错误 */
#define J_CONF_ERROR_MALFORMED 2    /* 格式错误 */

/* 获取错误代码和错误说明 */
int j_conf_errno(void);
const char *j_conf_strerror(void);


#endif
