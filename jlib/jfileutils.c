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

#include "jfileutils.h"
#include "jstrfuncs.h"
#include "jmem.h"
#include <stdlib.h>
#include <string.h>
#include <glob.h>

 /*
  * Checks to see if the path is absolute
  */
jint j_path_is_absolute(const jchar * path)
{
    return j_str_has_prefix(path, "/");
}

/*
 * Returns a pointer into file_name after the root component,
 * i.e. after the "/" in UNIX or "C:\" under Windows.
 * If file_name is not an absolute path it returns path it self.
 */
const jchar *j_path_skip_root(const jchar * path)
{
    if (!j_path_is_absolute(path)) {
        return path;
    }
    return path + 1;
}

/*
 * Expands all symbolic links and resolves references to /./, /../
 * and extra '/' characters in the null-terminated string
 * named by path to produce a canonicalized  absolute  pathname
 */
jchar *j_path_realpath(const jchar * path)
{
    jchar *real = realpath(path, NULL);
    return real;
}

/*
 * Gets the last component of the filename
 * Returns a newly allocated string containing
 * the last component of the filename
 */
jchar *j_path_basename(const jchar * path)
{
    jchar *slash = strrchr(path, '/');
    if (slash == NULL) {        /* slash not found */
        return j_strdup(path);
    }
    if (*(slash + 1)) {
        return j_strdup(slash + 1);
    }
    jchar *ptr = slash - 1;
    while (ptr != path) {
        if (*ptr == '/') {
            break;
        }
        ptr--;
    }
    if (*ptr == '/') {
        ptr++;
    }
    return j_strndup(ptr, slash - ptr);
}

/*
 * Searches for all the pathnames matching pattern accoding to the rules
 * used by shell (see glob(3)). No tilde expansion or parameter substitution
 * is done
 */
jchar **j_path_glob(const jchar * pattern)
{
    glob_t globs;
    jint ret = glob(pattern, 0, NULL, &globs);
    if (ret != 0) {
        return NULL;
    }
    jchar **strv =
        (jchar **) j_malloc(sizeof(jchar *) * (globs.gl_pathc + 1));
    jint i;
    for (i = 0; i < globs.gl_pathc; i++) {
        strv[i] = j_strdup(globs.gl_pathv[i]);
    }
    strv[i] = NULL;
    globfree(&globs);
    return strv;
}


/*
 * Joins two path, p2 must be relative
 */
jchar *j_path_join(const jchar * p1, const jchar * p2)
{
    if (j_path_is_absolute(p2)) {
        return NULL;
    }
    if (j_str_has_suffix(p1, "/")) {
        return j_strdup_printf("%s%s", p1, p2);
    }
    return j_strdup_printf("%s/%s", p1, p2);
}
