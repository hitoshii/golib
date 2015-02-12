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
#ifndef __J_LIB_STRFUNCS_H__
#define __J_LIB_STRFUNCS_H__

#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>

/*
 * Compares two strings. like standard strcmp
 * but comparing two NULL pointers returns 0
 */
int j_strcmp0(const char *s1, const char *s2);
int j_strncmp0(const char *s1, const char *s2, unsigned int count);

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
int j_strlen(const char *str);

/*
 * Removes trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchomp(char *str);


/*
 * Removes leading whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchug(char *str);


/*
 * Removes leading and trailing whitespace from a string
 */
#define j_strstrip(str) j_strchug(j_strchomp(str))

/*
 * Duplicates a string.
 * If str is NULL, returns NULL
 */
char *j_strdup(const char *str);


char *j_strdup_vprintf(const char *fmt, va_list vl);
char *j_strdup_printf(const char *fmt, ...);

/*
 * Duplicates the first count bytes of str.
 * Returns a newly-allocated buffer count+1 bytes long which always nul-terminated.
 * If str is less than count bytes long. the whole str is duplicated.
 */
char *j_strndup(const char *str, unsigned int count);

char *j_strnmdup(const char *str, unsigned int n, unsigned int m);

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
void j_strfreev(char **strv);


/*
 * Gets the length of array of strings
 */
int j_strv_length(char **strv);


/*
 * Checks whether the string str begins with prefix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_prefix(const char *str, const char *prefix);

/*
 * Checks whether the string str ends with suffix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_suffix(const char *str, const char *suffix);

/*
 * Checks whether str can be converted to a integer
 */
int j_str_isint(const char *str);
/*
 * Converts a string to a 64-bit integer
 * If str cannot be converted, returns -1;
 */
int64_t j_str_toint(const char *str);

/*
 * Creates a newly-allocated string from int
 */
char *j_str_fromint(int64_t num);

/*
 * Removes leading count bytes from a string
 * This function doesn't allocate or reallocate any memory
 * It modifies str in place.
 */
char *j_str_forward(char *str, unsigned int count);


/*
 */
char *j_str_replace(char *str, const char *t1, const char *t2);

/*
 * Splits str into a number of tokens not containing character c
 * The result is a NULL-terminated array of string. Use j_strfreev() to free it
 */
char **j_strsplit_c(const char *str, char c, int max);


#endif
