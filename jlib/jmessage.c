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
    }
    return j_log_handlers;
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
    if (handler) {
        handler->func = func;
        handler->user_data = user_data;
        return;
    }
    j_hash_table_insert(j_log_get_handlers(), j_strdup(domain),
                        j_log_handler_new(func, user_data));
}

void j_log_remove_handler(const jchar * domain)
{
    j_hash_table_remove_full(j_log_get_handlers(), (jpointer) domain);
}
