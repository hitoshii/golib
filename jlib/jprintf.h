/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
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


int j_printf_string_upper_bound(const char * format,
                                va_list args) J_GNUC_PRINTF(1, 0);


int j_printf(char const *format, ...) J_GNUC_PRINTF(1, 2);
int j_fprintf(FILE * file, char const *format, ...) J_GNUC_PRINTF(2, 3);
int j_sprintf(char * string, char const *format, ...) J_GNUC_PRINTF(2,
        3);
int j_snprintf(char * string, unsigned int size, char const *format,
               ...) J_GNUC_PRINTF(3, 4);
int j_vprintf(char const *format, va_list args) J_GNUC_PRINTF(1, 0);
int j_vfprintf(FILE * file, char const *format,
               va_list args) J_GNUC_PRINTF(2, 0);
int j_vsprintf(char * string, char const *format,
               va_list args) J_GNUC_PRINTF(2, 0);
int j_vsnprintf(char * string, unsigned int size, char const *format,
                va_list args) J_GNUC_PRINTF(3, 0);
int j_vasprintf(char ** string, char const *format,
                va_list args) J_GNUC_PRINTF(2, 0);


#endif
