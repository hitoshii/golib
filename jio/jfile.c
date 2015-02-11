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

#include "jfile.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jlib/jlib.h>


struct _JFile {
    int fd;
    char *path;
};

static inline void j_file_close(JFile * file);
static inline int j_file_open(JFile * file, int flags);
static inline int j_file_is_open(JFile * file);

const char *j_file_get_path(JFile * file)
{
    return file->path;
}

int j_file_get_unix_fd(JFile * file)
{
    return file->fd;
}

/*
 * This functions never fail
 */
JFile *j_file_new_for_path(const char *path)
{
    JFile *file = (JFile *) j_malloc(sizeof(JFile));
    file->path = j_strdup(path);
    file->fd = -1;
    return file;
}

int j_file_read(JFile * file, void *buf, unsigned int count)
{
    if (!j_file_is_open(file)) {
        if (!j_file_open(file, O_RDONLY)) {
            return 0;
        }
    }
    int n = read(j_file_get_unix_fd(file), buf, count);
    if (n == 0) {
        j_file_close(file);
    }
    return n;
}

char *j_file_readall(JFile * file)
{
    if (!j_file_open(file, O_RDONLY)) {
        return NULL;
    }
    JString *string = j_string_new();
    char buf[4096];
    int n;
    while ((n = j_file_read(file, buf, sizeof(buf) / sizeof(char))) > 0) {
        j_string_append_len(string, buf, n);
    }
    j_file_close(file);
    return j_string_free(string, 0);
}


void j_file_free(JFile * file)
{
    j_file_close(file);
    j_free(file->path);
    j_free(file);
}

static inline void j_file_close(JFile * file)
{
    if (file->fd >= 0) {
        close(file->fd);
        file->fd = -1;
    }
}

static inline int j_file_open(JFile * file, int flags)
{
    j_file_close(file);
    file->fd = open(j_file_get_path(file), flags, 0755);
    return file->fd >= 0;
}

static inline int j_file_is_open(JFile * file)
{
    return file->fd >= 0;
}
