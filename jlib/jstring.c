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

#include <string.h>

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
int j_strlen(char *str)
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
char *j_strdup(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
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
