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

#ifndef __JIO_FILE_H__
#define __JIO_FILE_H__

#include <jlib/jtypes.h>
#include <jlib/jobject.h>
#include <sys/mman.h>

typedef struct {
    JObject parent;
    jchar *path;
} JFile;

/* 该函数不会失败，除非内存分配出错 */
JFile *j_file_new(const jchar * path);

/* 打开文件描述符号 */
jint j_file_open_fd(JFile * f, jint mode);

/* 获取目录，new的时候指定的目录 */
const jchar *j_file_get_path(JFile * f);


/*
 * 内存映射，成功返回映射地址，失败返回NULL，如果成功，必须调用j_file_unmap()取消映射
 * @param prot PROT_EXEC PROT_READ PROT_WRITE PROT_NONE
 * @param flags MAP_SHARED MAP_PRIVATE
 * @param len len不能为NULL，如果*len为0，则映射整个文件，并返回文件长度，否则映射指定长度
 */
jchar *j_file_map(JFile * f, jint prot, jint flags, juint * len);
void j_file_unmap(jchar * addr, juint len);

#endif
