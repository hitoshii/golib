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
#ifndef __J_LIB_PATH_H__
#define __J_LIB_PATH_H__


/*
 * Checks to see if the path is absolute
 */
int j_path_is_absolute(const char *path);


#define j_path_is_relative(path)    (!(j_path_is_absolute(path)))


/*
 * Gets the last component of the filename
 * Returns a newly allocated string containing
 * the last component of the filename
 */
char *j_path_basename(const char *path);

/*
 * Expands all symbolic links and resolves references to /./, /../
 * and extra '/' characters in the null-terminated string
 * named by path to produce a canonicalized  absolute  pathname
 */
char *j_path_realpath(const char *path);

/*
 * Searches for all the pathnames matching pattern accoding to the rules
 * used by shell (see glob(3)). No tilde expansion or parameter substitution
 * is done
 */
char **j_path_glob(const char *pattern);

/*
 * Joins two path, p2 must be relative
 */
char *j_path_join(const char *p1, const char *p2);


#endif
