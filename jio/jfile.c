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

struct _JFile {
    jchar *path;
    jint ref;
};

/* 该函数不会失败，除非内存分配出错 */
JFile *j_file_new(const jchar * path)
{
    JFile *f = j_malloc(sizeof(JFile));
    f->path = j_strdup(path);
    f->ref = 1;

    return f;
}

static inline void j_file_free(JFile * f)
{
    j_free(f->path);
    j_free(f);
}

void j_file_ref(JFile * f)
{
    j_atomic_int_inc(&f->ref);
}

void j_file_unref(JFile * f)
{
    if (j_atomic_int_dec_and_test(&f->ref)) {
        j_file_free(f);
    }
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
