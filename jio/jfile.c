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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#include "jfile.h"
#include <jlib/jlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


static void j_file_free(JFile * file)
{
    j_free(file->path);
}

/* 该函数不会失败，除非内存分配出错 */
JFile *j_file_new(const jchar * path)
{
    JFile *file = j_malloc(sizeof(JFile));
    file->path = j_strdup(path);
    J_OBJECT_INIT(file, j_file_free);

    return file;
}

jint j_file_open_fd(JFile * f, jint mode)
{
    return open(f->path, mode);
}

/* 获取目录，new的时候指定的目录 */
const jchar *j_file_get_path(JFile * f)
{
    return f->path;
}

jchar *j_file_map(JFile * f, jint prot, jint flags, juint * len)
{
    int fd = j_file_open_fd(f, O_RDWR);
    if (fd < 0) {
        return NULL;
    }
    if (*len == 0) {
        struct stat buf;
        if (fstat(fd, &buf)) {
            close(fd);
            return NULL;
        }
        *len = buf.st_size;
    }
    void *addr = mmap(NULL, *len, prot, flags, fd, 0);
    close(fd);
    return (jchar *) addr;
}

void j_file_unmap(jchar * addr, juint len)
{
    munmap(addr, len);
}
