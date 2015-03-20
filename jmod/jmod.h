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
#ifndef __J_MOD_H__
#define __J_MOD_H__

#include "struct.h"
#include <jio/jio.h>
#include <stdarg.h>

/*
 * Loads a module from path
 * Returns NULL on error
 */
JModule *j_mod_load(const char *location, const char *path);


typedef void (*JModuleVLog) (JLogLevel level, const char *fmt, va_list ap);
void j_mod_set_log_func(JModuleVLog func);
void j_mod_log(JLogLevel level, const char *fmt, ...);


#endif
