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


typedef struct {
    const char *name;
    JHookStruct *hooks;
} JacModule;


#define JACQUES_MODULE_NAME  "__jacques_module__"
#define JACUQES_MODULE(object) JacModule* JACQUES_MODULE_NAME = &object


/* 从模块中读取模块结构 */
JacModule *jacques_loads_module(const char *filename);

#endif
