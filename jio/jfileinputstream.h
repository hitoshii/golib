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
#ifndef __JIO_FILE_INPUT_STREAM_H__
#define __JIO_FILE_INPUT_STREAM_H__

#include "jinputstream.h"
#include "jfile.h"

typedef struct {
    JInputStream parent;
    jint fd;
} JFileInputStream;

/* 打开文件读，失败返回NULL */
JFileInputStream *j_file_read(JFile * f);


#endif
