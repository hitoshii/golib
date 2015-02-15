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
#ifndef __J_IO_FILE_H__
#define __J_IO_FILE_H__

#include <sys/stat.h>

/*
 * Reads all data from path
 * Returns newly-allocated string on success
 * Returns NULL on error, and standard errno will be set
 */
char *j_file_readall(const char *path);


typedef enum {
    J_FILE_TYPE_SCK = S_IFSOCK, /* socket */
    J_FILE_TYPE_LNK = S_IFLNK,  /* symbolic link */
    J_FILE_TYPE_REG = S_IFREG,  /* regular file */
    J_FILE_TYPE_BLK = S_IFBLK,  /* block device */
    J_FILE_TYPE_DIR = S_IFDIR,  /* directory */
    J_FILE_TYPE_CHR = S_IFCHR,  /* character device */
    J_FILE_TYPE_IFO = S_IFIFO,  /* FIFO */
} JFileType;

typedef struct {
    JFileType type;
} JFileInfo;
#define j_file_is_reg(info) S_ISREG((info).type)
#define j_file_is_dir(info) S_ISDIR((info).type)

/*
 * Checks to see if path is an existing regular file
 */
int j_file_exists(const char *path);

int j_file_query_info(const char *path, JFileInfo * info);



#endif
