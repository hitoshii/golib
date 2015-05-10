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
#ifndef __JLIB_MESSAGE_H__
#define __JLIB_MESSAGE_H__

#include "jtypes.h"
#include <stdarg.h>

typedef enum {
    J_LOG_LEVEL_ERROR = 1 << 0,
    J_LOG_LEVEL_CRITICAL = 1 << 1,
    J_LOG_LEVEL_WARNING = 1 << 2,
    J_LOG_LEVEL_MESSAGE = 1 << 3,
    J_LOG_LEVEL_INFO = 1 << 4,
    J_LOG_LEVEL_DEBUG = 1 << 5,

    J_LOG_LEVEL_MASK = 0xFF
} JLogLevelFlag;

#define J_LOG_LEVEL_USER_SHIFT (6)

typedef void (*JLogFunc) (const jchar * domain, JLogLevelFlag level,
                          const jchar * message, jpointer user_data);

/*
 * Sets the log handler for a domain and a set of log levels.
 */
void j_log_set_handler(const jchar * domain,
                       JLogFunc func, jpointer user_data);
void j_log_remove_handler(const jchar * domain);


void j_logv(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
            va_list ap) J_GNUC_PRINTF(3, 0);
void j_log(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
           ...) J_GNUC_PRINTF(3, 4);

#endif
