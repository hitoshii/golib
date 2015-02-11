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


typedef struct _JFile JFile;

/*
 * This functions never fail
 */
JFile *j_file_new_for_path(const char *path);


const char *j_file_get_path(JFile * file);

int j_file_read(JFile * file, void *buf, unsigned int count);
char *j_file_readall(JFile * file);

void j_file_free(JFile * file);


#endif
