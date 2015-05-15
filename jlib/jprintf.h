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
#ifndef __JLIB_PRINTF_H__
#define __JLIB_PRINTF_H__

#include "jtypes.h"
#include <stdio.h>


#define _j_printf    printf
#define _j_fprintf   fprintf
#define _j_sprintf   sprintf
#define _j_snprintf  snprintf

#define _j_vprintf   vprintf
#define _j_vfprintf  vfprintf
#define _j_vsprintf  vsprintf
#define _j_vsnprintf vsnprintf


jint j_printf_string_upper_bound(const jchar * format,
                                 va_list args) J_GNUC_PRINTF(1, 0);


jint j_printf(jchar const *format, ...) J_GNUC_PRINTF(1, 2);
jint j_fprintf(FILE * file, jchar const *format, ...) J_GNUC_PRINTF(2, 3);
jint j_sprintf(jchar * string, jchar const *format, ...) J_GNUC_PRINTF(2,
                                                                       3);
jint j_snprintf(jchar * string, juint size, jchar const *format,
                ...) J_GNUC_PRINTF(3, 4);
jint j_vprintf(jchar const *format, va_list args) J_GNUC_PRINTF(1, 0);
jint j_vfprintf(FILE * file, jchar const *format,
                va_list args) J_GNUC_PRINTF(2, 0);
jint j_vsprintf(jchar * string, jchar const *format,
                va_list args) J_GNUC_PRINTF(2, 0);
jint j_vsnprintf(jchar * string, juint size, jchar const *format,
                 va_list args) J_GNUC_PRINTF(3, 0);
jint j_vasprintf(jchar ** string, jchar const *format,
                 va_list args) J_GNUC_PRINTF(2, 0);


#endif
