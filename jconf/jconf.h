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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#ifndef __JCONF_CONF_H__
#define __JCONF_CONF_H__

/**
 * JConf是类似JSON的数据格式，用于配置文件。它与JSON的主要不同有如下几点
 * 1. JConf不需要一个根对象
 * 2. JConf的键值不需要用双引号，同时键值的命名有一定规范，
 * 3. 两个结点之间使用分号(;)或者换行符(\n)分割
 *
 * 对于下面JSON数据
 * {
 *  "name": "john",
 *  "age": 13,
 *  "email": ["john@gmail.com", "john@outlook.com"],
 *  "extra": {"city": "Tokyo", "twitter": "johnn"}
 * }
 *
 * 其对应的JConf格式如下
 * name: "john"
 * age: 13
 * email: ["john@gmail.com"; "john@outlook.com"]
 * extra: {city: "Tokyo"; twitter: "johnn"}
 *
 * 当然，你可以把多行合并和拆分
 * name: "john";age: 13; email["john@gmail.com"; "john@outlook.com"]
 * extra: {
 *      city: "Tokyo"
 *      twitter: "johnn"
 * }
 */

#include "jconfnode.h"

#endif
