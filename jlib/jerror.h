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
#ifndef __JLIB_ERROR_H__
#define __JLIB_ERROR_H__


#include <stdarg.h>
#include "jquark.h"


/* 获取系统调用的错误码 */
jint j_errno(void);
jboolean j_strerror(jint errnum, jchar * buf, juint buflen);


typedef struct {
    JQuark domain;
    jint code;
    jchar *message;
} JError;


JError *j_error_new(JQuark domain, jint code,
                    const jchar * format, ...) J_GNUC_PRINTF(3, 4);
JError *j_error_new_literal(JQuark domain, jint code,
                            const jchar * message);
JError *j_error_new_valist(JQuark domain, jint code, const jchar * format,
                           va_list args) J_GNUC_PRINTF(3, 0);
void j_error_free(JError * error);
JError *j_error_copy(const JError * error);
jboolean j_error_matches(const JError * error, JQuark domain, jint code);

/*
 * 如果err是NULL，则不做任何改变直接返回
 * 如果err不是NULL，那*err必须为NULL，将创建一个新的JError并附给*err
 */
void j_set_error(JError ** err, JQuark domain, jint code,
                 const jchar * format, ...) J_GNUC_PRINTF(4, 5);
void j_set_error_literal(JError ** error, JQuark domain,
                         jint code, const jchar * message) J_GNUC_PRINTF(4,
                                                                         0);

/*
 * 如果dest是NULL，则释放src
 * 否则将src赋值给*dest，原来的*dest必须为NULL
 * 调用该函数后src将不再可用
 */
void j_propagate_error(JError ** dest, JError * src);

/*
 * 如果err和*err都不为空，则释放*err，并设置*err=NULL
 */
void j_clear_error(JError ** err);

/*
 * 如果err和*err都不为空，则将格式化的字符串添加到 ->message中
 */
void j_prefix_error(JError ** err, const jchar * format,
                    ...) J_GNUC_PRINTF(2, 3);

/*
 * 先j_propagate_error()再j_prefix_error()
 */
void j_propagate_prefixed_error(JError ** dest, JError * src,
                                const jchar * format, ...) J_GNUC_PRINTF(3,
                                                                         4);


#endif
