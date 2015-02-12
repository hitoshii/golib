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

#include "jpath.h"
#include "jstrfuncs.h"
#include <stdlib.h>

 /*
  * Checks to see if the path is absolute
  */
int j_path_is_absolute(const char *path)
{
    return j_str_has_prefix(path, "/");
}


/*
 * Joins two path, p2 must be relative
 */
char *j_path_join(const char *p1, const char *p2)
{
    if (j_path_is_absolute(p2)) {
        return NULL;
    }
    if (j_str_has_suffix(p1, "/")) {
        return j_strdup_printf("%s%s", p1, p2);
    }
    return j_strdup_printf("%s/%s", p1, p2);
}
