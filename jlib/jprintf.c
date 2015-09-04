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
#include "jprintf.h"
#include "jmem.h"
#include <stdarg.h>

int j_printf_string_upper_bound(const char * format, va_list args) {
    char c;
    return _j_vsnprintf(&c, 1, format, args) + 1;
}

int j_printf(char const *format, ...) {
    va_list args;
    int retval;

    va_start(args, format);
    retval = j_vprintf(format, args);
    va_end(args);
    return retval;
}

int j_fprintf(FILE * file, char const *format, ...) {
    va_list args;
    int retval;

    va_start(args, format);
    retval = j_vfprintf(file, format, args);
    va_end(args);
    return retval;
}

int j_sprintf(char * string, char const *format, ...) {
    va_list args;
    int retval;

    va_start(args, format);
    retval = j_vsprintf(string, format, args);
    va_end(args);
    return retval;
}

int j_snprintf(char * string, unsigned int size, char const *format, ...) {
    va_list args;
    int retval;

    va_start(args, format);
    retval = j_vsnprintf(string, size, format, args);
    va_end(args);
    return retval;
}

int j_vprintf(char const *format, va_list args) {
    return _j_vprintf(format, args);
}

int j_vfprintf(FILE * file, char const *format, va_list args) {
    return _j_vfprintf(file, format, args);
}

int j_vsnprintf(char * string, unsigned int size, char const *format,
                va_list args) {
    return _j_vsnprintf(string, size, format, args);
}

int j_vsprintf(char * string, char const *format, va_list args) {
    return _j_vsprintf(string, format, args);
}

int j_vasprintf(char ** string, char const *format, va_list args) {
    int len;
#if defined(HAVE_VASPRINTF)
    len = vasprintf(string, format, args);
    if (len < 0) {
        *string = NULL;
    }
#else
    va_list args2;
    va_copy(args2, args);
    *string = j_new(char, j_printf_string_upper_bound(format, args));
    len = _j_vsprintf(*string, format, args2);
    va_end(args2);
#endif
    return len;
}
