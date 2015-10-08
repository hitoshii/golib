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
#include "jmod.h"
#include <jlib/jlib.h>



/* 从模块中读取模块结构 */
JacModule *jacques_loads_module(const char *filename) {
    JModule *mod = j_module_open(filename, J_MODULE_NODELETE|J_MODULE_LAZY);
    if(mod==NULL) {
        printf("2\n");
        return NULL;
    }
    void *ptr=NULL;
    JacModule *result = NULL;
    if(!j_module_symbol(mod, JACQUES_MODULE_NAME, &ptr)||ptr==NULL) {
        printf("1\n");
        goto OUT;
    }
    result = (JacModule*)*((JacModule**)ptr);
OUT:
    j_module_close(mod);
    return result;
}
