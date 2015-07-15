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
#ifndef __JLIB_QUARK_H__
#define __JLIB_QUARK_H__


#include "jtypes.h"


typedef juint32 JQuark;

/*
 * JQuark 实现字符串和一个整数的唯一映射
 * 使用j_quark_from_string()或者j_quark_from_static_string()从字符串创建JQuark
 */

/*
 * string可以为NULL
 * 根据string得到一个JQuark
 * 如果string为NULL或者没有对应的JQuark，则返回0
 * 如果你想要JQuark不存在时自动生成，使用j_quark_from_string()
 * 或者j_quark_from_static_string()
 */
JQuark j_quark_try_string(const jchar * string);
JQuark j_quark_from_static_string(const jchar * string);
JQuark j_quark_from_string(const jchar * string);
const jchar *j_quark_to_string(JQuark quark) J_GNUC_CONST;

const jchar *j_internal_string(const jchar * string);
const jchar *j_internal_static_string(const jchar * string);


#endif
