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


typedef enum {
    J_SOURCE_FLAG_ACTIVE = 1 << 0,
    J_SOURCE_FLAG_IN_CALL = 1 << 1,
    J_SOURCE_FLAG_BLOCKED = 1 << 2,
    J_SOURCE_FLAG_READY = 1 << 3,
    J_SOURCE_FLAG_CAN_RECURSE = 1 << 4,
    J_SOURCE_FLAG_MASK = 0xFF,
} JSourceFlag;

#define J_PRIORITY_HIGH -100
#define J_PRIORITY_DEFAULT 0
#define J_PRIORITY_HIGH_IDLE 100
#define J_PRIORITY_DEFAULT_IDLE 200
#define J_PRIORITY_LOW 300


const jchar *j_source_get_name(JSource * src);

/*
 * Creates a new JSource structure.
 * The size is specified to allow createing structures derived from JSource that contain additional data.
 * The size must be greater than sizeof(JSource)
 */
JSource *j_source_new(JSourceFuncs * funcs, juint struct_size);


/*
 * Decreases the reference count of a source by one.
 * If the resulting reference count is zero the source and associated memory will be destroyed.
 */
void j_source_unref(JSource * src);

/*
 * Removes a source from its GMainContext, if any, and mark it as destroyed.
 * The source cannot be subsequently added to another context.
 * It is safe to call this on sources which have already been removed from their context.
 */
void j_source_destroy(JSource * src);

/* JMainContext */
typedef struct _JMainContext JMainContext;


#endif
