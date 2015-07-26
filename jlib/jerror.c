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
#include "jerror.h"
#include "jmessage.h"
#include "jmem.h"
#include "jstrfuncs.h"
#include <errno.h>
#include <string.h>

/* 获取系统调用的错误码 */
jint j_errno(void)
{
    return errno;
}

jboolean j_strerror(jint errnum, jchar * buf, juint buflen)
{
    return strerror_r(errnum, buf, buflen) == 0;
}
