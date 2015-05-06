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
#ifndef __JLIB_MAIN_H__
#define __JLIB_MAIN_H__
#include "jtypes.h"

/*
 * Queries the system monotonic time.
 */
jint64 j_get_monotonic_time(void);

/* JSource */
typedef jboolean(*JSourceFunc) (jpointer user_data);

typedef struct _JSourceCallbackFuncs JSourceCallbackFuncs;
typedef struct _JSourceFuncs JSourceFuncs;
typedef struct _JSource JSource;

/* JMainContext */
typedef struct _JMainContext JMainContext;


#endif
