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
#define _GNU_SOURCE
#include "jstrfuncs.h"
#include "jstring.h"
#include "jmem.h"
#include <string.h>
#include <stdio.h>

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
 * Compares two strings. like standard strcmp
 * but comparing two NULL pointers returns 0
 */
int j_strcmp0(const char *s1, const char *s2)
{
    if (s1 == NULL && s2 == NULL) {
        return 0;
    } else if (s1 == NULL && s2 != NULL) {
        return 1;
    } else if (s1 != NULL && s2 == NULL) {
        return -1;
    }
    return strcmp(s1, s2);
}

int j_strncmp0(const char *s1, const char *s2, unsigned int count)
{
    if (s1 == NULL && s2 == NULL) {
        return 0;
    } else if (s1 == NULL && s2 != NULL) {
        return 1;
    } else if (s1 != NULL && s2 == NULL) {
        return -1;
    }
    return strncmp(s1, s2, count);
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

char *j_strdup_vprintf(const char *fmt, va_list vl)
{
    char *strp = NULL;
    if (vasprintf(&strp, fmt, vl) < 0) {
        return NULL;
    }
    return strp;
}

char *j_strdup_printf(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    char *str = j_strdup_vprintf(fmt, vl);
    va_end(vl);
    return str;
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
    buffer[count] = '\0';
    return buffer;
}

char *j_strnmdup(const char *str, unsigned int n, unsigned int m)
{
    if (str == NULL || n > m) {
        return NULL;
    }
    return j_strndup(str + n, m - n);
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


/*
 * Gets the length of array of strings
 */
int j_strv_length(char **strv)
{
    if (strv == NULL) {
        return -1;
    }
    int len = 0;
    char **ptr = strv;
    while (*ptr) {
        len++;
        ptr++;
    }
    return len;
}

/*
 * Checks whether the string str begins with prefix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_prefix(const char *str, const char *prefix)
{
    const char *p1 = str;
    const char *p2 = prefix;
    while (*p1 != '\0' && *p2 != '\0') {
        if (*p1 != *p2) {
            return 0;
        }
        p1++;
        p2++;
    }
    if (*p2 == '\0') {
        return 1;
    }
    return 0;
}

/*
 * Checks whether the string str ends with suffix
 * Returns 1 if yes, otherwise 0
 */
int j_str_has_suffix(const char *str, const char *suffix)
{
    int str_len = j_strlen(str);
    int suffix_len = j_strlen(suffix);
    if (str_len < suffix_len) {
        return 0;
    }
    return j_strcmp0(str + str_len - suffix_len, suffix) == 0;
}

/*
 * Checks whether str can be converted to a integer
 */
int j_str_isint(const char *str)
{
    if (!(*str == '-' || isdigit(*str))) {
        return 0;
    }
    str++;
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/*
 * Converts a string to a 64-bit integer
 */
int64_t j_str_toint(const char *str)
{
    int sign = 0;
    if (*str == '-') {
        sign = 1;
        str++;
    }
    int64_t result = 0;
    int i, len = j_strlen(str);
    for (i = len - 1; i >= 0; i--) {
        char c = str[i];
        int n = (c - '0');
        if (n < 0 || n > 9) {
            return -1;
        }
        int k = len - i - 1;
        int j = 1;
        while (k > 0) {
            j *= 10;
            k--;
        }
        result += n * j;
    }
    if (sign) {
        result = -result;
    }
    return result;
}

/*
 * Creates a newly-allocated string from int
 */
char *j_str_fromint(int64_t num)
{
    char buf[64];
    snprintf(buf, sizeof(buf) / sizeof(char), "%ld", num);
    return j_strdup(buf);
}

/*
 * Removes leading count bytes from a string
 * This function doesn't allocate or reallocate any memory
 * It modifies str in place.
 */
char *j_str_forward(char *str, unsigned int count)
{
    if (count == 0) {
        return str;
    }
    char *ptr = str;
    unsigned int offset = 0;
    while (*ptr) {
        offset++;
        ptr++;
        if (offset == count) {
            break;
        }
    }
    unsigned int pos = 0;
    while (*ptr) {
        *(str + pos) = *ptr;
        ptr++;
        pos++;
    }
    *(str + pos) = '\0';
    return str;
}

char *j_str_replace(char *str, const char *t1, const char *t2)
{
    char *start = str;
    char *ptr = strstr(start, t1);
    int len = j_strlen(t1);
    JString *string = NULL;
    while (ptr) {
        if (string == NULL) {
            string = j_string_new();
        }
        j_string_append_len(string, start, ptr - start);
        j_string_append(string, t2);
        start = ptr + len;
        ptr = strstr(start, t1);
    }
    if (string == NULL) {
        return str;
    }
    j_string_append(string, start);
    j_free(str);
    return j_string_free(string, 0);
}

/*
 * Splits str into a number of tokens not containing character c
 * The result is a NULL-terminated array of string. Use j_strfreev() to free it
 */
char **j_strsplit_c(const char *str, char c, int max)
{
    max--;
    const char *ptr = str;
    unsigned int len = 0;
    while (*ptr) {
        if (*ptr == c) {
            len++;
        }
        ptr++;
    }
    if (max >= 0 && max < len) {
        len = max;
    }
    char **strv = (char **) j_malloc(sizeof(char *) * (len + 2));
    unsigned int pos = 0;
    const char *start = str;
    ptr = start;
    while (*ptr) {
        if (*ptr == c && len > 0) {
            len--;
            strv[pos++] = j_strndup(start, ptr - start);
            start = ptr + 1;
        }
        ptr++;
    }
    strv[pos++] = j_strndup(start, ptr - start);
    strv[pos] = NULL;
    return strv;
}

static inline char *j_str_encode_utf8(const char *str, int strict)
{
    JString *string = j_string_new();
    const char *ptr = str;
    while (*ptr) {
        unsigned char c1 = *ptr++, c2, c3, c4;
        unsigned int point = ' ';
        if (c1 < 0x80) {
            j_string_append_c(string, c1);
            continue;
        } else if (c1 < 0xC0) {
            goto ERROR;
        } else if (c1 < 0xE0) {
            c2 = *ptr++;
            if ((c2 & 0xC0) != 0x80) {
                goto ERROR;
            }
            point = (c1 << 6) + c2 - 0x3080;
        } else if (c1 < 0xF0) {
            c2 = *ptr++;
            if ((c2 & 0xC0) != 0x80 || (c1 == 0xE0 && c2 < 0xA0)) {
                goto ERROR;
            }
            c3 = *ptr++;
            if ((c3 & 0xC0) != 0x80) {
                goto ERROR;
            }
            point = (c1 << 12) + (c2 << 6) + c3 - 0xE2080;
        } else if (c1 < 0xF5) {
            c2 = *ptr++;
            if ((c2 & 0xC0) != 0x80 || (c1 == 0xF0 && c2 < 0x90)
                || (c1 == 0xF4 && c2 >= 0x90)) {
                goto ERROR;
            }
            c3 = *ptr++;
            if ((c3 & 0xC0) != 0x80) {
                goto ERROR;
            }
            c4 = *ptr++;
            if ((c4 & 0xC0) != 0x80) {
                goto ERROR;
            }
            point = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4 - 0x3C82080;
        } else {
            goto ERROR;
        }
        j_string_append_printf(string, "\\u%04X", point);
        continue;
      ERROR:
        if (strict) {
            j_string_free(string, 1);
            return NULL;
        }
    }
    return j_string_free(string, 0);
}

char *j_str_encode(const char *str, JEncoding encoding, int strict)
{
    if (encoding == J_ENCODING_UTF8) {
        return j_str_encode_utf8(str, strict);
    }
    return NULL;
}
