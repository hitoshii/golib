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
#include "jconf.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define ERROR_MAX    1024

static char gError[ERROR_MAX];

/*
 * 获取错误说明
 */
const char *j_conf_get_error(void)
{
    return gError;
}

static inline int j_conf_root_parse(JConfRoot * root, const char *data);

/*
 * 从一个文件中载入配置
 */
JConfRoot *j_conf_load_from_file(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    struct stat sbuf;
    if (fstat(fd, &sbuf) < 0) {
        close(fd);
        return NULL;
    }
    unsigned int size = sbuf.st_size;
    char *data =
        (char *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
                      0);
    close(fd);
    if (data == MAP_FAILED) {   /* MAP_FAILED = (void*)-1 */
        return NULL;
    }
    data[size++] = '\0';
    JConfRoot *root = j_conf_root_new(path);
    if (!j_conf_root_parse(root, data)) {
        j_conf_root_free(root);
        root = NULL;
    }
    munmap(data, size);
    return root;
}

static inline int j_conf_root_parse(JConfRoot * root, const char *data)
{
    return 0;
}
