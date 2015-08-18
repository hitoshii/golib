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
#ifndef __JLIB_MODULE_H__
#define __JLIB_MODULE_H__

#include "jtypes.h"
#include <dlfcn.h>

#define J_MODULE_SUFFIX "so"

typedef enum {
    J_MODULE_LAZY=RTLD_LAZY,
    J_MODULE_NOW=RTLD_NOW,
    J_MODULE_GLOBAL=RTLD_GLOBAL,
    J_MODULE_LOCAL=RTLD_LOCAL,
    J_MODULE_NODELETE=RTLD_NODELETE,
    J_MODULE_NOLOAD=RTLD_NOLOAD,
    J_MODULE_DEEPBIND=RTLD_DEEPBIND,
} JModuleFlags;

typedef struct _JModule JModule;

JModule *j_module_open(const jchar *filepath, JModuleFlags flags);

const jchar *j_module_name(JModule *mod);

/* NULL也是一个有效的符号 */
jboolean j_module_symbol(JModule *module, const jchar *symbol_name, jpointer *symbol);

void j_module_close(JModule *module);


#endif
