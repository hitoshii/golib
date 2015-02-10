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

#include "jstring.h"
#include "jmem.h"
#include <string.h>

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
int j_strlen(const char *str)
{
    if (str == NULL) {
        return -1;
    }
    return strlen(str);
}

/*
 * Duplicates a string.
 * If str is NULL, returns NULL
 */
char *j_strdup(const char *str)
{
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}

/*
 * Duplicates the first count bytes of str.
 * Returns a newly-allocated buffer count+1 bytes long which always nul-terminated.
 * If str is less than count bytes long. the whole str is duplicated.
 */
char *j_strndup(const char *str, unsigned int count)
{
    if (str == NULL) {
        return NULL;
    }
    int len = j_strlen(str);
    if (len <= count) {
        return j_strdup(str);
    }
    char *buffer = j_malloc(sizeof(char) * (count + 1));
    strncpy(buffer, str, count);
    return buffer;
}


/*
 * Removes trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchomp(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    char *wp = NULL;
    char *ptr = str;
    while (*ptr != '\0') {
        if (wp == NULL) {
            if (*ptr == ' ') {
                wp = ptr;
            }
        } else if (*ptr != ' ') {
            wp = NULL;
        }
        ptr++;
    }
    if (wp) {
        *wp = '\0';
    }
    return str;
}

/*
 * Removes leading whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
char *j_strchug(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    int wp = 0;
    char *ptr = str;
    while (*ptr != '\0') {
        if (*ptr == ' ') {
            wp++;
        } else {
            break;
        }
        ptr++;
    }
    if (wp) {
        ptr = str;
        while (*(ptr + wp) != '\0') {
            *ptr = *(ptr + wp);
            ptr++;
        }
        *(ptr + wp) = '\0';
    }
    return str;
}

/*
 * Creates a NULL-terminated array of strings. makeing a use of va_list
 */
char **j_strdupv_valist(unsigned int count, va_list vl)
{
    if (count == 0) {
        return NULL;
    }
    char **strv = (char **) j_malloc(sizeof(char *) * (count + 1));
    unsigned int i;
    for (i = 0; i < count; i++) {
        const char *s = va_arg(vl, char *);
        strv[i] = j_strdup(s);
    }
    strv[count] = NULL;
    return strv;
}

/*
 * Creates a NULL-terminated array of strings, making a use of va_list
 * This function doesn't duplicate the strings, just take them
 */
char **j_strv_valist(unsigned int count, va_list vl)
{
    if (count == 0) {
        return NULL;
    }
    char **strv = (char **) j_malloc(sizeof(char *) * (count + 1));
    unsigned int i;
    for (i = 0; i < count; i++) {
        strv[i] = va_arg(vl, char *);
    }
    strv[count] = NULL;
    return strv;
}

/*
 * Creates a NULL-terminated array of strings, like j_strdupv()
 * But this function doesn't duplicate strings, it just takes the strings
 */
char **j_strv(unsigned int count, ...)
{
    va_list vl;
    va_start(vl, count);
    char **strv = j_strv_valist(count, vl);
    va_end(vl);
    return strv;
}

/*
 * Creates a NULL-terminated array of strings
 */
char **j_strdupv(unsigned int count, ...)
{
    va_list vl;
    va_start(vl, count);
    char **strv = j_strdupv_valist(count, vl);
    va_end(vl);
    return strv;
}

/*
 * Fres a NULL-terminated array of strings.
 * If strv is NULL, do nothing
 */
void j_strfreev(char **strv)
{
    if (strv == NULL) {
        return;
    }
    char **ptr = strv;
    while (*ptr != NULL) {
        j_free(*ptr);
        ptr++;
    }
    j_free(strv);
}
