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

#define J_PATH_SEPARATOR '/'
#define j_is_path_separator(c) ((c)==J_PATH_SEPARATOR)

/*
 * Checks to see if the path is absolute
 */
boolean j_path_is_absolute(const char * path);

/*
 * Returns a pointer into file_name after the root component,
 * i.e. after the "/" in UNIX or "C:\" under Windows.
 * If file_name is not an absolute path it returns path it self.
 */
const char *j_path_skip_root(const char * path);


#define j_path_is_relative(path)    (!(j_path_is_absolute(path)))


/*
 * Gets the last component of the filename
 * Returns a newly allocated string containing
 * the last component of the filename
 */
char *j_path_basename(const char * path);

/*
 * j_path_dirname:
 * @path:
 * Gets the directory components of a file name
 * Returns: a new allocated string
 */
char *j_path_dirname(const char *path);

/*
 * Expands all symbolic links and resolves references to /./, /../
 * and extra '/' characters in the null-terminated string
 * named by path to produce a canonicalized  absolute  pathname
 */
char *j_path_realpath(const char * path);

/*
 * Searches for all the pathnames matching pattern accoding to the rules
 * used by shell (see glob(3)). No tilde expansion or parameter substitution
 * is done
 */
char **j_path_glob(const char * pattern);

/*
 * Joins two path, p2 must be relative
 */
char *j_path_join(const char * p1, const char * p2);


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
int j_mkdir_with_parents(const char *pathname, int mode);


typedef enum {
    J_FILE_TEST_IS_REGULAR = 1<<0,
    J_FILE_TEST_IS_SYMLINK = 1<<1,
    J_FILE_TEST_IS_DIR = 1<<2,
    J_FILE_TEST_IS_EXECUTABLE = 1<<3,
    J_FILE_TEST_IS_BLOCK=1<<4,
    J_FILE_TEST_EXISTS = 1<<5,
} JFileTest;

/* 可以合并多个测试选项，J_FILE_TEST_EXISTS总是被测试的 */
boolean j_file_test(const char *path, JFileTest test);


#endif
