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

#include "jmessage.h"
#include "jthread.h"
#include "jhashtable.h"
#include "jmem.h"
#include "jstrfuncs.h"
#include "jprintf.h"
#include <stdio.h>

typedef struct {
    JLogFunc func;
    jpointer user_data;
} JLogHandler;


static inline JLogHandler *j_log_handler_new(JLogFunc func,
                                             jpointer user_data)
{
    JLogHandler *handler = (JLogHandler *) j_malloc(sizeof(JLogHandler));
    handler->func = func;
    handler->user_data = user_data;
    return handler;
}

static void j_log_handler_free(JLogHandler * handler)
{
    j_free(handler);
}

static JHashTable *j_log_handlers = NULL;

static inline JHashTable *j_log_get_handlers(void)
{
    if (J_UNLIKELY(j_log_handlers == NULL)) {
        j_log_handlers =
            j_hash_table_new(8, j_str_hash, j_str_equal, j_free,
                             (JDestroyNotify) j_log_handler_free);
        j_hash_table_insert(j_log_handlers, J_LOG_DOMAIN,
                            j_log_handler_new(j_log_default_handler,
                                              NULL));
    }
    return j_log_handlers;
}

void j_log_default_handler(const jchar * domain, JLogLevelFlag flag,
                           const jchar * message, jpointer user_data)
{
    const jchar *level = NULL;
    switch (flag & J_LOG_LEVEL_MASK) {
    case J_LOG_LEVEL_CRITICAL:
        level = "Critical";
        break;
    case J_LOG_LEVEL_ERROR:
        level = "Error";
        break;
    case J_LOG_LEVEL_WARNING:
        level = "Warning";
        break;
    case J_LOG_LEVEL_MESSAGE:
        level = "Message";
        break;
    case J_LOG_LEVEL_INFO:
        level = "Info";
        break;
    default:
        level = "Debug";
        break;
    }
    j_printf("%s: %s\n", level, message);
}

J_LOCK_DEFINE_STATIC(j_message_lock);


static inline JLogHandler *j_log_find_handler(const jchar * domain)
{
    JLogHandler *handler;
    J_LOCK(j_message_lock);
    handler = j_hash_table_find(j_log_get_handlers(), domain);
    J_UNLOCK(j_message_lock);
    return handler;
}

/*
 * Sets the log handler for a domain and a set of log levels.
 */
void j_log_set_handler(const jchar * domain,
                       JLogFunc func, jpointer user_data)
{
    JLogHandler *handler = j_log_find_handler(domain);
    J_LOCK(j_message_lock);
    if (handler) {
        handler->func = func;
        handler->user_data = user_data;
    } else {
        j_hash_table_insert(j_log_get_handlers(), j_strdup(domain),
                            j_log_handler_new(func, user_data));
    }
    J_UNLOCK(j_message_lock);
}

void j_log_remove_handler(const jchar * domain)
{
    J_LOCK(j_message_lock);
    j_hash_table_remove_full(j_log_get_handlers(), (jpointer) domain);
    J_UNLOCK(j_message_lock);
}

void j_logv(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
            va_list ap)
{
    JLogHandler *handler = j_log_find_handler(domain);
    if (handler == NULL || handler->func == NULL) {
        return;
    }
    jchar buf[4096];            /* 一次输出最长4096 */
    vsnprintf(buf, sizeof(buf) / sizeof(jchar), msg, ap);
    handler->func(domain, flag, buf, handler->user_data);
}

void j_log(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
           ...)
{
    va_list ap;
    va_start(ap, msg);
    j_logv(domain, flag, msg, ap);
    va_end(ap);
}
