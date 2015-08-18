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
#ifndef __J_LIB_FILEUTILS_H__
#define __J_LIB_FILEUTILS_H__

#include "jtypes.h"

/*
 * Checks to see if the path is absolute
 */
jint j_path_is_absolute(const jchar * path);

/*
 * Returns a pointer into file_name after the root component,
 * i.e. after the "/" in UNIX or "C:\" under Windows.
 * If file_name is not an absolute path it returns path it self.
 */
const jchar *j_path_skip_root(const jchar * path);


#define j_path_is_relative(path)    (!(j_path_is_absolute(path)))


/*
 * Gets the last component of the filename
 * Returns a newly allocated string containing
 * the last component of the filename
 */
jchar *j_path_basename(const jchar * path);

/*
 * j_path_dirname:
 * @path:
 * Gets the directory components of a file name
 * Returns: a new allocated string
 */
jchar *j_path_dirname(const jchar *path);

/*
 * Expands all symbolic links and resolves references to /./, /../
 * and extra '/' characters in the null-terminated string
 * named by path to produce a canonicalized  absolute  pathname
 */
jchar *j_path_realpath(const jchar * path);

/*
 * Searches for all the pathnames matching pattern accoding to the rules
 * used by shell (see glob(3)). No tilde expansion or parameter substitution
 * is done
 */
jchar **j_path_glob(const jchar * pattern);

/*
 * Joins two path, p2 must be relative
 */
jchar *j_path_join(const jchar * p1, const jchar * p2);


/*
 * j_mkdir_with_parents:
 * @pathname: a pathname
 * @mode: permissions to use for newly created directories
 *
 * Create a directory if it doesn't already exist. Create itermediate
 * parent directories as needed, too
 *
 * Returns: 0 if the directory already exists, or was successfully created.
 *          Returns -1 on error, with errno set.
 */
jint j_mkdir_with_parents(const jchar *pathname, int mode);


#endif
