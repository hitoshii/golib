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
#ifndef __JIO_LOG_H__
#define __JIO_LOG_H__

typedef enum {
    J_LOG_LEVEL_DEBUG,
    J_LOG_LEVEL_VERBOSE,
    J_LOG_LEVEL_WARNING,
    J_LOG_LEVEL_ERROR,
} JLogLevel;

typedef struct {
    int fd;
    char *fmt;
} JLogger;

/*
 * log format
 *      %l log level
 *      %0 log time
 *      %m log message
 */
JLogger *j_logger_open(const char *path, const char *fmt);

void j_logger_log(JLogger * logger, JLogLevel level, const char *message);
#define j_logger_warning(logger,message)    \
            j_logger_log(logger,J_LOG_LEVEL_WARNING,message)
#define j_logger_error(logger,message)  \
            j_logger_log(logger,J_LOG_LEVEL_ERROR,message)

void j_logger_close(JLogger * logger);


#endif
