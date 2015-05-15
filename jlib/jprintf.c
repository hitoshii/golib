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
#include "jprintf.h"
#include "jmem.h"
#include <stdarg.h>

jint j_printf_string_upper_bound(const jchar * format, va_list args)
{
    jchar c;
    return _j_vsnprintf(&c, 1, format, args) + 1;
}

jint j_printf(jchar const *format, ...)
{
    va_list args;
    jint retval;

    va_start(args, format);
    retval = j_vprintf(format, args);
    va_end(args);
    return retval;
}

jint j_fprintf(FILE * file, jchar const *format, ...)
{
    va_list args;
    jint retval;

    va_start(args, format);
    retval = j_vfprintf(file, format, args);
    va_end(args);
    return retval;
}

jint j_sprintf(jchar * string, jchar const *format, ...)
{
    va_list args;
    jint retval;

    va_start(args, format);
    retval = j_vsprintf(string, format, args);
    va_end(args);
    return retval;
}

jint j_snprintf(jchar * string, juint size, jchar const *format, ...)
{
    va_list args;
    jint retval;

    va_start(args, format);
    retval = j_snprintf(string, size, format, args);
    va_end(args);
    return retval;
}

jint j_vprintf(jchar const *format, va_list args)
{
    return _j_vprintf(format, args);
}

jint j_vfprintf(FILE * file, jchar const *format, va_list args)
{
    return _j_vfprintf(file, format, args);
}

jint j_vsnprintf(jchar * string, juint size, jchar const *format,
                 va_list args)
{
    return _j_vsnprintf(string, size, format, args);
}

jint j_vsprintf(jchar * string, jchar const *format, va_list args)
{
    return _j_vsprintf(string, format, args);
}

jint j_vasprintf(jchar ** string, jchar const *format, va_list args)
{
    jint len;
#if defined(HAVE_VASPRINTF)
    len = vasprintf(string, format, args);
    if (len < 0) {
        *string = NULL;
    }
#else
    va_list args2;
    va_copy(args2, args);
    *string = j_new(jchar, j_printf_string_upper_bound(format, args));
    len = _j_vsprintf(*string, format, args2);
    va_end(args2);
#endif
    return len;
}
