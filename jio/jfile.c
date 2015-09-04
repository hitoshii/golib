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
#include "jfile.h"
#include <jlib/jlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


static void j_file_free(JFile * file) {
    j_free(file->path);
}

/* 该函数不会失败，除非内存分配出错 */
JFile *j_file_new(const char * path) {
    JFile *file = j_malloc(sizeof(JFile));
    file->path = j_strdup(path);
    J_OBJECT_INIT(file, j_file_free);

    return file;
}

int j_file_open_fd(JFile * f, int mode) {
    return open(f->path, mode);
}

/* 获取目录，new的时候指定的目录 */
const char *j_file_get_path(JFile * f) {
    return f->path;
}

char *j_file_map(JFile * f, int prot, int flags, unsigned int * len) {
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
    return (char *) addr;
}

void j_file_unmap(char * addr, unsigned int len) {
    munmap(addr, len);
}
