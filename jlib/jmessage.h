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
juint j_log_set_handler(const jchar *domain, JLogLevelFlag level,
                        JLogFunc func, jpointer user_data);
juint j_log_set_handler_full(const jchar *domain, JLogLevelFlag level,
                             JLogFunc func, jpointer user_data, JDestroyNotify destroy);

void j_log_remove_handler(const jchar * domain, juint id);


void j_logv(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
            va_list ap) J_GNUC_PRINTF(3, 0);
void j_log(const jchar * domain, JLogLevelFlag flag, const jchar * msg,
           ...) J_GNUC_PRINTF(3, 4);

#define J_LOG_DOMAIN    ("")

#define j_error(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_ERROR,__VA_ARGS__)
#define j_critical(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_CRITICAL,__VA_ARGS__)
#define j_warning(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_WARNING,__VA_ARGS__)
#define j_message(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_MESSAGE,__VA_ARGS__)
#define j_info(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_INFO,__VA_ARGS__)
#define j_debug(...) j_log(J_LOG_DOMAIN,J_LOG_LEVEL_DEBUG,__VA_ARGS__)


#define STRINGIFY(x) STRINGIFY_ARG(x)
#define STRINGIFY_ARG(x) #x

#define J_LOGAT __FILE__ ":" STRINGIFY(__LINE__)

void j_return_if_fail_warning(const jchar * domain, const jchar * at,
                              const jchar * expression);

#define j_return_if_fail(expr)  J_STMT_START\
        if(J_UNLIKELY(!(expr))){\
            j_return_if_fail_warning(J_LOG_DOMAIN,J_LOGAT,#expr);\
            return;\
        }J_STMT_END

#define j_return_val_if_fail(expr, val)  J_STMT_START\
        if(J_UNLIKELY(!(expr))){\
            j_return_if_fail_warning(J_LOG_DOMAIN,J_LOGAT,#expr);\
            return val;\
        }J_STMT_END

#endif
