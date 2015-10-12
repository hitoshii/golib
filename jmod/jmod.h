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
#ifndef __JMOD_H__
#define __JMOD_H__

#include "jhook.h"
#include "jproxy.h"
#include <jlib/jlib.h>

typedef struct {
    const char *name;
    JacHook *hooks;
} JacModule;


#define JACQUES_MODULE_NAME  "__jacques_module__"
#define JACQUES_MODULE(object) JacModule *__jacques_module__ = &object


JList *get_jacques_modules(void);

/* 从模块中读取模块结构 */
JacModule *jac_loads_module(const char *filename);

boolean jac_loads_modules(JList *filenames);

#endif
