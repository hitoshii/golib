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
#include "jdir.h"
#include "jfile.h"
#include <jlib/jlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/*
 * Checks to see if path is a existing directory
 */
int j_dir_exist(const char *path)
{
    JFileInfo info;
    if (!j_file_query_info(path, &info)) {
        return 0;
    }
    return j_file_is_dir(info);
}

int j_mkdir_with_parents(const char *path, int mode)
{
    if (j_dir_exist(path)) {
        return 1;
    }

    char *fn = j_strdup(path);
    char *ptr = (char *) j_path_skip_root(fn);
    while (*ptr) {
        if (*ptr == '/') {
            *ptr = '\0';
            if (mkdir(fn, mode) && errno != EEXIST) {
                j_free(fn);
                return 0;
            }
            *ptr = '/';
        }
        ptr++;
    }
    j_free(fn);
    mkdir(path, mode);
    return j_dir_exist(path);
}
