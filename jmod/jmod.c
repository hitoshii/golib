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
#include "jmod.h"
#include <jlib/jlib.h>
#include <jio/jio.h>
#include <stdio.h>
#include <dlfcn.h>


static const char *get_module_path(const char *MODULE_LOCATION,
                                   const char *name)
{
    static char buf[2048];
    if (j_path_is_absolute(name)) {
        snprintf(buf, sizeof(buf) / sizeof(char), "%s", name);
        if (j_file_exists(buf)) {
            return buf;
        }
    } else {
        snprintf(buf, sizeof(buf) / sizeof(char), "%s/%s", MODULE_LOCATION,
                 name);
        if (j_file_exists(buf)) {
            return buf;
        }
        snprintf(buf, sizeof(buf) / sizeof(char), "%s/%s.so",
                 MODULE_LOCATION, name);
        if (j_file_exists(buf)) {
            return buf;
        }
    }
    return NULL;
}


/*
 * Loads a module from path
 * Returns NULL on error
 */
JModule *j_mod_load(const char *MODULE_LOCATION, const char *path)
{
    path = get_module_path(MODULE_LOCATION, path);
    if (path == NULL) {
        return NULL;
    }
    void *dl = dlopen(path, RTLD_NOW);
    if (dl == NULL) {
        return NULL;
    }
    JModule *mod = (JModule *) dlsym(dl, "module_struct");
    dlclose(dl);
    if (mod == NULL) {
        return NULL;
    }
    return mod;
}
