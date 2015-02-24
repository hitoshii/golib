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

#include "jlog.h"
#include <jlib/jlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


JLogger *j_logger_open(const char *path, const char *fmt)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0755);
    if (fd < 0) {
        return NULL;
    }

    JLogger *logger = (JLogger *) j_malloc(sizeof(JLogger));
    logger->fd = fd;
    logger->fmt = j_strdup(fmt);

    return logger;
}


static inline const char *j_logger_levelstr(JLogLevel level)
{
    switch (level) {
    case J_LOG_LEVEL_DEBUG:
        return "DEBUG";
    case J_LOG_LEVEL_VERBOSE:
        return "VERBOSE";
    case J_LOG_LEVEL_WARNING:
        return "WARNING";
    case J_LOG_LEVEL_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}


/*
 * log format
 *      %l log level
 *      %0 log date time
 *      %m log message
 */
void j_logger_log(JLogger * logger, JLogLevel level, const char *message)
{
    JString *buf = j_string_new();
    const char *ptr = logger->fmt;
    int flag = 0;
    time_t t = time(NULL);
    struct tm dt;
    localtime_r(&t, &dt);

    char tbuf[64];
    while (*ptr) {
        char c = *ptr;
        if (flag == 0) {
            if (c == '%') {
                flag = 1;
            } else {
                j_string_append_c(buf, c);
            }
        } else {
            if (c == 'm') {
                j_string_append(buf, message);
            } else if (c == 'l') {
                j_string_append(buf, j_logger_levelstr(level));
            } else if (c == '0') {
                strftime(tbuf, sizeof(tbuf) / sizeof(char), "%x-%X", &dt);
                j_string_append(buf, tbuf);
            } else if (c == '%') {
                j_string_append_c(buf, '%');
            }
            flag = 0;
        }
        ptr++;
    }
    j_string_append_c(buf, '\n');
    unsigned int len = buf->len;
    char *data = j_string_free(buf, 0);
    write(logger->fd, data, len);
    j_free(data);
}

void j_logger_close(JLogger * logger)
{
    close(logger->fd);
    j_free(logger->fmt);
    j_free(logger);
}
