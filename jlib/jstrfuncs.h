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

JLIB_VAR const uint16_t *const j_ascii_table;

#define j_ascii_istype(c, t)    \
    ((j_ascii_table[(unsigned char) (c)] & t) != 0)

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
int j_strcmp0(const char * s1, const char * s2);
int j_strncmp0(const char * s1, const char * s2, unsigned int count);

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
int j_strlen(const char * str);

/*
 * Removes trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchomp(char * str);


/*
 * Removes leading whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchug(char * str);


/*
 * Removes leading and trailing whitespace from a string
 */
#define j_strstrip(str) j_strchug(j_strchomp(str))

/*
 * Duplicates a string.
 * If str is NULL, returns NULL
 */
char *j_strdup(const char * str);


char *j_strdup_vprintf(const char * fmt, va_list vl);
char *j_strdup_printf(const char * fmt, ...);

/*
 * Duplicates the first count bytes of str.
 * Returns a newly-allocated buffer count+1 bytes long which always nul-terminated.
 * If str is less than count bytes long. the whole str is duplicated.
 */
char *j_strndup(const char * str, unsigned int count);

char *j_strnmdup(const char * str, unsigned int n, unsigned int m);

char *j_stpcpy(char * dest, const char * src);
char *j_strconcat(const char * string1, ...) J_GNUC_MALLOC;

/*
 * Creates a NULL-terminated array of strings, which has count strings.
 * This function duplicates the strings.
 * For instance, j_strdupv(3,"hello","world","!")
 * Do not use NULL as a argument.
 */
char **j_strdupv(unsigned int count, ...);

/*
 * Creates a NULL-terminated array of strings. makeing a use of va_list
 * This function duplicates the strings.
 */
char **j_strdupv_valist(unsigned int count, va_list vl);

/*
 * Creates a NULL-terminated array of strings, like j_strdupv()
 * But this function doesn't duplicate strings, it just takes the strings
 */
char **j_strv(unsigned int count, ...);

/*
 * Creates a NULL-terminated array of strings, making a use of va_list
 * This function doesn't duplicate the strings, just take them
 */
char **j_strv_valist(unsigned int count, va_list vl);

/*
 * Fres a NULL-terminated array of strings.
 * If strv is NULL, do nothing
 */
void j_strfreev(char ** strv);


/*
 * Gets the length of array of strings
 */
int j_strv_length(char ** strv);


/*
 * Checks whether the string str begins with prefix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_prefix(const char * str, const char * prefix);

/*
 * Checks whether the string str ends with suffix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_suffix(const char * str, const char * suffix);

/*
 * Checks whether str can be converted to a integer
 */
int j_str_isint(const char * str);
/*
 * Converts a string to a 64-bit integer
 * If str cannot be converted, returns -1;
 */
int64_t j_str_toint(const char * str);

/*
 * Removes leading count bytes from a string
 * This function doesn't allocate or reallocate any memory
 * It modifies str in place.
 */
char *j_str_forward(char * str, unsigned int count);


/*
 */
char *j_str_replace(char * str, const char * t1, const char * t2);

/*
 * Splits str into a number of tokens not containing character c
 * The result is a NULL-terminated array of string. Use j_strfreev() to free it
 */
char **j_strsplit_c(const char * str, char c, int max);


typedef enum {
    J_ENCODING_UTF8,
} JEncoding;
/*
 * Encode str using the codec registered for encoding.
 * If strict is true. NULL may be returnd because of encoding error.
 * Otherwise A new allocated string is returned.
 */
char *j_str_encode(const char * str, JEncoding encoding,
                   boolean strict);

#endif
