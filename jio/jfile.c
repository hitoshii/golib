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

/*
 * Reads all data from path
 * Returns newly-allocated string on success
 * Returns NULL on error, and standard errno will be set
 */
char *j_file_readall(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    JString *string = j_string_new();
    char buf[4096];
    int n;
    while ((n = read(fd, buf, sizeof(buf) / sizeof(char))) > 0) {
        j_string_append_len(string, buf, n);
    }
    close(fd);
    if (n != 0) {
        j_string_free(string, 1);
        return NULL;
    }
    return j_string_free(string, 0);
}

/*
 * Checks to see if path is a existing normal file
 */
int j_file_exists(const char *path)
{
    struct stat buf;
    if (stat(path, &buf) != 0) {
        return 0;
    }
    return S_ISREG(buf.st_mode);
}
