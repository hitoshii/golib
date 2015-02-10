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
#ifndef __J_LIB_STRING_H__
#define __J_LIB_STRING_H__

#include <stdarg.h>

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

/*
 * Duplicates the first count bytes of str.
 * Returns a newly-allocated buffer count+1 bytes long which always nul-terminated.
 * If str is less than count bytes long. the whole str is duplicated.
 */
char *j_strndup(const char *str, unsigned int count);

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


#endif
