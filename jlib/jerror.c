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
#include "jerror.h"
#include "jmessage.h"
#include "jmem.h"
#include "jstrfuncs.h"


JError *j_error_new(JQuark domain, jint code, const jchar * format, ...)
{
    j_return_val_if_fail(format != NULL && domain != 0, NULL);

    va_list args;
    va_start(args, format);
    JError *error = j_error_new_valist(domain, code, format, args);
    va_end(args);
    return error;
}

JError *j_error_new_literal(JQuark domain, jint code,
                            const jchar * message)
{
    j_return_val_if_fail(message != NULL && domain != 0, NULL);

    JError *error = j_malloc(sizeof(JError));
    error->domain = domain;
    error->code = code;
    error->message = j_strdup(message);

    return error;
}

JError *j_error_new_valist(JQuark domain, jint code,
                           const jchar * format, va_list args)
{
    j_return_val_if_fail(format != NULL && domain != 0, NULL);

    JError *error = j_malloc(sizeof(JError));
    error->domain = domain;
    error->code = code;
    error->message = j_strdup_vprintf(format, args);

    return error;
}

void j_error_free(JError * error)
{
    j_return_if_fail(error != NULL);

    j_free(error->message);
    j_free(error);
}

JError *j_error_copy(const JError * error)
{
    j_return_val_if_fail(error != NULL, NULL);
    j_return_val_if_fail(error->message != NULL
                         && error->domain != 0, NULL);

    JError *copy = j_malloc(sizeof(JError));

    copy->domain = error->domain;
    copy->code = error->code;
    copy->message = j_strdup(error->message);

    return copy;
}

jboolean j_error_matches(const JError * error, JQuark domain, jint code)
{
    return error && error->domain == domain && error->code == code;
}

#define ERROR_OVERWRITEN_WARNING "JError set over the top of a previous GError or uninitialized memory.\n" \
                "This indicates a bug in someone's code. You must ensure an error is NULL before it's set.\n" \
                "The overwriting error message was: %s"

/*
 * 如果err是NULL，则不做任何改变直接返回
 * 如果err不是NULL，那*err必须为NULL，将创建一个新的JError并附给*err
 */
void j_set_error(JError ** err, JQuark domain, jint code,
                 const jchar * format, ...)
{
    j_return_if_fail(err == NULL);

    va_list args;
    va_start(args, format);
    JError *new = j_error_new_valist(domain, code, format, args);
    va_end(args);

    if (*err == NULL) {
        *err = new;
    } else {
        j_warning(ERROR_OVERWRITEN_WARNING, new->message);
        j_error_free(new);
    }
}

void j_set_error_literal(JError ** error, JQuark domain,
                         jint code, const jchar * message)
{
    j_return_if_fail(error != NULL);

    if (*error == NULL) {
        *error = j_error_new_literal(domain, code, message);
    } else {
        j_warning(ERROR_OVERWRITEN_WARNING, message);
    }
}

/*
 * 如果dest是NULL，则释放src
 * 否则将src赋值给*dest，原来的*dest必须为NULL
 * 调用该函数后src将不再可用
 */
void j_propagate_error(JError ** dest, JError * src)
{
    j_return_if_fail(src != NULL);

    if (dest == NULL) {
        if (src) {
            j_error_free(src);
        }
        return;
    } else {
        if (*dest != NULL) {
            j_warning(ERROR_OVERWRITEN_WARNING, src->message);
            j_error_free(src);
        } else {
            *dest = src;
        }
    }
}

/*
 * 如果err和*err都不为空，则释放*err，并设置*err=NULL
 */
void j_clear_error(JError ** err)
{
    if (err && *err) {
        j_error_free(*err);
        *err = NULL;
    }
}

J_GNUC_PRINTF(2, 0)
static inline void j_error_add_prefix(jchar ** string,
                                      const jchar * format, va_list args)
{
    jchar *prefix = j_strdup_vprintf(format, args);
    jchar *oldstring = *string;
    *string = j_strconcat(prefix, oldstring, NULL);
    j_free(oldstring);
    j_free(prefix);
}

/*
 * 如果err和*err都不为空，则将格式化的字符串添加到 ->message中
 */
void j_prefix_error(JError ** err, const jchar * format, ...)
{
    if (err && *err) {
        va_list args;
        va_start(args, format);
        j_error_add_prefix(&(*err)->message, format, args);
        va_end(args);
    }
}

/*
 * 先j_propagate_error()再j_prefix_error()
 */
void j_propagate_prefixed_error(JError ** dest, JError * src,
                                const jchar * format, ...)
{
    j_propagate_error(dest, src);

    if (dest && *dest) {
        va_list args;
        va_start(args, format);
        j_error_add_prefix(&(*dest)->message, format, args);
        va_end(args);
    }
}
