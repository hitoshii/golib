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
#ifndef __J_LIB_STRFUNCS_H__
#define __J_LIB_STRFUNCS_H__

#include <stdarg.h>
#include "jtypes.h"

typedef enum {
    J_ASCII_ALNUM = 1 << 0,
    J_ASCII_ALPHA = 1 << 1,
    J_ASCII_CNTRL = 1 << 2,
    J_ASCII_DIGIT = 1 << 3,
    J_ASCII_GRAPH = 1 << 4,
    J_ASCII_LOWER = 1 << 5,
    J_ASCII_PRINT = 1 << 6,
    J_ASCII_PUNCT = 1 << 7,
    J_ASCII_SPACE = 1 << 8,
    J_ASCII_UPPER = 1 << 9,
    J_ASCII_XDIGIT = 1 << 10,
} JAsciiType;

JLIB_VAR const juint16 *const j_ascii_table;

#define j_ascii_istype(c, t)    \
    ((j_ascii_table[(juchar) (c)] & t) != 0)

#define j_ascii_isalnum(c)  j_ascii_istype(c, J_ASCII_ALNUM)
#define j_ascii_isalpha(c)  j_ascii_istype(c, J_ASCII_ALPHA)
#define j_ascii_iscntrl(c)  j_ascii_istype(c, J_ASCII_CNTRL)
#define j_ascii_isdigit(c)  j_ascii_istype(c, J_ASCII_DIGIT)
#define j_ascii_isgraph(c)  j_ascii_istype(c, J_ASCII_GRAPH)
#define j_ascii_islower(c)  j_ascii_istype(c, J_ASCII_LOWER)
#define j_ascii_isprint(c)  j_ascii_istype(c, J_ASCII_PRINT)
#define j_ascii_ispunct(c)  j_ascii_istype(c, J_ASCII_PUNCT)
#define j_ascii_isspace(c)  j_ascii_istype(c, J_ASCII_SPACE)
#define j_ascii_isupper(c)  j_ascii_istype(c, J_ASCII_UPPER)
#define j_ascii_isxdigit(c) j_ascii_istype(c, J_ASCII_XDIGIT)

/*
 * Compares two strings. like standard strcmp
 * but comparing two NULL pointers returns 0
 */
jint j_strcmp0(const jchar * s1, const jchar * s2);
jint j_strncmp0(const jchar * s1, const jchar * s2, juint count);

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
int j_strlen(const jchar * str);

/*
 * Removes trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
jchar *j_strchomp(jchar * str);


/*
 * Removes leading whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
jchar *j_strchug(jchar * str);


/*
 * Removes leading and trailing whitespace from a string
 */
#define j_strstrip(str) j_strchug(j_strchomp(str))

/*
 * Duplicates a string.
 * If str is NULL, returns NULL
 */
jchar *j_strdup(const jchar * str);


jchar *j_strdup_vprintf(const jchar * fmt, va_list vl);
jchar *j_strdup_printf(const jchar * fmt, ...);

/*
 * Duplicates the first count bytes of str.
 * Returns a newly-allocated buffer count+1 bytes long which always nul-terminated.
 * If str is less than count bytes long. the whole str is duplicated.
 */
jchar *j_strndup(const jchar * str, juint count);

jchar *j_strnmdup(const jchar * str, juint n, juint m);

jchar *j_stpcpy(jchar * dest, const jchar * src);
jchar *j_strconcat(const jchar * string1, ...) J_GNUC_MALLOC;

/*
 * Creates a NULL-terminated array of strings, which has count strings.
 * This function duplicates the strings.
 * For instance, j_strdupv(3,"hello","world","!")
 * Do not use NULL as a argument.
 */
jchar **j_strdupv(juint count, ...);

/*
 * Creates a NULL-terminated array of strings. makeing a use of va_list
 * This function duplicates the strings.
 */
jchar **j_strdupv_valist(juint count, va_list vl);

/*
 * Creates a NULL-terminated array of strings, like j_strdupv()
 * But this function doesn't duplicate strings, it just takes the strings
 */
jchar **j_strv(juint count, ...);

/*
 * Creates a NULL-terminated array of strings, making a use of va_list
 * This function doesn't duplicate the strings, just take them
 */
jchar **j_strv_valist(juint count, va_list vl);

/*
 * Fres a NULL-terminated array of strings.
 * If strv is NULL, do nothing
 */
void j_strfreev(jchar ** strv);


/*
 * Gets the length of array of strings
 */
int j_strv_length(jchar ** strv);


/*
 * Checks whether the string str begins with prefix
 * Returns 1 if yes, otherwise 0
 */
jint j_str_has_prefix(const jchar * str, const jchar * prefix);

/*
 * Checks whether the string str ends with suffix
 * Returns 1 if yes, otherwise 0
 */
jint j_str_has_suffix(const jchar * str, const jchar * suffix);

/*
 * Checks whether str can be converted to a integer
 */
jint j_str_isint(const jchar * str);
/*
 * Converts a string to a 64-bit integer
 * If str cannot be converted, returns -1;
 */
jint64 j_str_toint(const jchar * str);

/*
 * Removes leading count bytes from a string
 * This function doesn't allocate or reallocate any memory
 * It modifies str in place.
 */
jchar *j_str_forward(jchar * str, juint count);


/*
 */
jchar *j_str_replace(jchar * str, const jchar * t1, const jchar * t2);

/*
 * Splits str into a number of tokens not containing character c
 * The result is a NULL-terminated array of string. Use j_strfreev() to free it
 */
jchar **j_strsplit_c(const jchar * str, jchar c, jint max);


typedef enum {
    J_ENCODING_UTF8,
} JEncoding;
/*
 * Encode str using the codec registered for encoding.
 * If strict is true. NULL may be returnd because of encoding error.
 * Otherwise A new allocated string is returned.
 */
jchar *j_str_encode(const jchar * str, JEncoding encoding,
                    jboolean strict);

#endif
