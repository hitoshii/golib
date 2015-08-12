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
#define _GNU_SOURCE
#include "jstrfuncs.h"
#include "jstring.h"
#include "jmem.h"
#include "jmessage.h"
#include <string.h>
#include <stdio.h>

static const juint16 ascii_table_data[256] = {
    0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
    0x004, 0x104, 0x104, 0x004, 0x104, 0x104, 0x004, 0x004,
    0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
    0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
    0x140, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
    0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
    0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
    0x459, 0x459, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
    0x0d0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
    0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
    0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
    0x253, 0x253, 0x253, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
    0x0d0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
    0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
    0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
    0x073, 0x073, 0x073, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x004
    /* the upper 128 are all zeroes */
};

const juint16 *const j_ascii_table = ascii_table_data;

/*
 * Calculates the length of a string
 * If str is NULL, -1 is returned
 */
jint j_strlen(const jchar * str) {
    if (str == NULL) {
        return -1;
    }
    return strlen(str);
}

/*
 * Compares two strings. like standard strcmp
 * but comparing two NULL pointers returns 0
 */
jint j_strcmp0(const jchar * s1, const jchar * s2) {
    if (s1 == NULL && s2 == NULL) {
        return 0;
    } else if (s1 == NULL && s2 != NULL) {
        return 1;
    } else if (s1 != NULL && s2 == NULL) {
        return -1;
    }
    return strcmp(s1, s2);
}

jint j_strncmp0(const jchar * s1, const jchar * s2, juint count) {
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
jchar *j_strdup(const jchar * str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}

jchar *j_strdup_vprintf(const jchar * fmt, va_list vl) {
    char *strp = NULL;
    if (vasprintf(&strp, fmt, vl) < 0) {
        return NULL;
    }
    return strp;
}

jchar *j_strdup_printf(const jchar * fmt, ...) {
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
jchar *j_strndup(const jchar * str, juint count) {
    if (str == NULL) {
        return NULL;
    }
    jint len = j_strlen(str);
    if (len <= count) {
        return j_strdup(str);
    }
    jchar *buffer = j_malloc(sizeof(jchar) * (count + 1));
    strncpy(buffer, str, count);
    buffer[count] = '\0';
    return buffer;
}

jchar *j_strnmdup(const jchar * str, juint n, juint m) {
    if (str == NULL || n > m) {
        return NULL;
    }
    return j_strndup(str + n, m - n);
}

jchar *j_stpcpy(jchar * dest, const jchar * src) {
    j_return_val_if_fail(dest != NULL && src != NULL, NULL);
#ifndef HAVE_STPCPY
    return stpcpy(dest, src);
#else
    jchar *d = dest;
    const jchar *s = src;

    do {
        *d++ = *s;
    } while (*s++ != '\0');
    return d - 1;
#endif
}

jchar *j_strconcat(const jchar * string1, ...) {
    j_return_val_if_fail(string1 != NULL, NULL);

    jsize l = 1 + j_strlen(string1);

    va_list args;
    va_start(args, string1);
    jchar *s = va_arg(args, jchar *);
    while (s) {
        l += strlen(s);
        s = va_arg(args, jchar *);
    }
    va_end(args);

    jchar *concat = (jchar *) j_new(jchar, l);
    jchar *ptr = concat;

    ptr = j_stpcpy(ptr, string1);
    va_start(args, string1);
    s = va_arg(args, jchar *);
    while (s) {
        ptr = j_stpcpy(ptr, s);
        s = va_arg(args, jchar *);
    }
    va_end(args);

    return concat;
}

/*
 * Removes trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it modifies str in
 * place. Therefore, it cannot be used on statically allocated strings.
 * If str is NULL, returns NULL
 */
jchar *j_strchomp(jchar * str) {
    if (str == NULL) {
        return NULL;
    }
    jchar *wp = NULL;
    jchar *ptr = str;
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
jchar *j_strchug(jchar * str) {
    if (str == NULL) {
        return NULL;
    }
    jint wp = 0;
    jchar *ptr = str;
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
jchar **j_strdupv_valist(juint count, va_list vl) {
    if (J_UNLIKELY(count == 0)) {
        return NULL;
    }
    jchar **strv = (jchar **) j_malloc(sizeof(jchar *) * (count + 1));
    juint i;
    for (i = 0; i < count; i++) {
        const jchar *s = va_arg(vl, jchar *);
        strv[i] = j_strdup(s);
    }
    strv[count] = NULL;
    return strv;
}

/*
 * Creates a NULL-terminated array of strings, making a use of va_list
 * This function doesn't duplicate the strings, just take them
 */
jchar **j_strv_valist(juint count, va_list vl) {
    if (J_UNLIKELY(count == 0)) {
        return NULL;
    }
    jchar **strv = (jchar **) j_malloc(sizeof(jchar *) * (count + 1));
    juint i;
    for (i = 0; i < count; i++) {
        strv[i] = va_arg(vl, jchar *);
    }
    strv[count] = NULL;
    return strv;
}

/*
 * Creates a NULL-terminated array of strings, like j_strdupv()
 * But this function doesn't duplicate strings, it just takes the strings
 */
jchar **j_strv(juint count, ...) {
    va_list vl;
    va_start(vl, count);
    jchar **strv = j_strv_valist(count, vl);
    va_end(vl);
    return strv;
}

/*
 * Creates a NULL-terminated array of strings
 */
jchar **j_strdupv(juint count, ...) {
    va_list vl;
    va_start(vl, count);
    jchar **strv = j_strdupv_valist(count, vl);
    va_end(vl);
    return strv;
}

/*
 * Fres a NULL-terminated array of strings.
 * If strv is NULL, do nothing
 */
void j_strfreev(jchar ** strv) {
    if (strv == NULL) {
        return;
    }
    jchar **ptr = strv;
    while (*ptr != NULL) {
        j_free(*ptr);
        ptr++;
    }
    j_free(strv);
}


/*
 * Gets the length of array of strings
 */
jint j_strv_length(jchar ** strv) {
    if (strv == NULL) {
        return -1;
    }
    jint len = 0;
    jchar **ptr = strv;
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
jint j_str_has_prefix(const jchar * str, const jchar * prefix) {
    const jchar *p1 = str;
    const jchar *p2 = prefix;
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
jint j_str_has_suffix(const jchar * str, const jchar * suffix) {
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
jint j_str_isint(const jchar * str) {
    if (!(*str == '-' || j_ascii_isdigit(*str))) {
        return 0;
    }
    str++;
    while (*str) {
        if (!j_ascii_isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/*
 * Converts a string to a 64-bit integer
 */
jint64 j_str_toint(const jchar * str) {
    jint sign = 0;
    if (*str == '-') {
        sign = 1;
        str++;
    }
    jint64 result = 0;
    jint i, len = j_strlen(str);
    for (i = len - 1; i >= 0; i--) {
        jchar c = str[i];
        jint n = (c - '0');
        if (n < 0 || n > 9) {
            return -1;
        }
        jint k = len - i - 1;
        jint j = 1;
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
 * Removes leading count bytes from a string
 * This function doesn't allocate or reallocate any memory
 * It modifies str in place.
 */
jchar *j_str_forward(jchar * str, juint count) {
    if (count == 0) {
        return str;
    }
    jchar *ptr = str;
    juint offset = 0;
    while (*ptr) {
        offset++;
        ptr++;
        if (offset == count) {
            break;
        }
    }
    juint pos = 0;
    while (*ptr) {
        *(str + pos) = *ptr;
        ptr++;
        pos++;
    }
    *(str + pos) = '\0';
    return str;
}

jchar *j_str_replace(jchar * str, const jchar * t1, const jchar * t2) {
    jchar *start = str;
    jchar *ptr = strstr(start, t1);
    jint len = j_strlen(t1);
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
jchar **j_strsplit_c(const jchar * str, jchar c, jint max) {
    max--;
    const jchar *ptr = str;
    juint len = 0;
    while (*ptr) {
        if (*ptr == c) {
            len++;
        }
        ptr++;
    }
    if (max >= 0 && max < len) {
        len = max;
    }
    jchar **strv = (jchar **) j_malloc(sizeof(jchar *) * (len + 2));
    juint pos = 0;
    const jchar *start = str;
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

static inline jchar *j_str_encode_utf8(const jchar * str, jboolean strict) {
    JString *string = j_string_new();
    const jchar *ptr = str;
    while (*ptr) {
        juchar c1 = *ptr++, c2, c3, c4;
        juint point = ' ';
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

jchar *j_str_encode(const jchar * str, JEncoding encoding, jboolean strict) {
    if (encoding == J_ENCODING_UTF8) {
        return j_str_encode_utf8(str, strict);
    }
    return NULL;
}
