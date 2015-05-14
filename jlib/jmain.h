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
#include "jepoll.h"
#include "jthread.h"

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
juint j_source_get_id(JSource * src);
JMainContext *j_source_get_context(JSource * src);

/*
 * Creates a new JSource structure.
 * The size is specified to allow createing structures derived from JSource that contain additional data.
 * The size must be greater than sizeof(JSource)
 */
JSource *j_source_new(JSourceFuncs * funcs, juint struct_size);

/*
 * Monitors fd for the IO events in events .
 */
void j_source_add_poll_fd(JSource * src, jint fd, juint io);


/*
 * Increases the reference count on a source by one.
 */
void j_source_ref(JSource * src);
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


/*
 * Adds a GSource to a context so that it will be executed within that context.
 * Returns the source ID
 */
juint j_source_attach(JSource * src, JMainContext * ctx);

/*
 * Creates a new JMainContext
 */
JMainContext *j_main_context_new(void);

/*
 * Returns the global default main context. 
 */
JMainContext *j_main_context_default(void);

/*
 * Increases the reference count on a context by one
 */
void j_main_context_ref(JMainContext * ctx);

/*
 * Decreases the reference
 */
void j_main_context_unref(JMainContext * ctx);

/*
 * Tries to become the owner of the specified context.
 * If some other thread is the owner of the context, returns FALSE immediately. 
 * Ownership is properly recursive: the owner can require ownership again and will release ownership when g_main_context_release() is called as many times as g_main_context_acquire().
 */
jboolean j_main_context_acquire(JMainContext * ctx);


/*
 * Releases ownership of a context previously acquired by this thread with g_main_context_acquire().
 * If the context was acquired multiple times, the ownership will be released only when g_main_context_release() is called as many times as it was acquired.
 */
void j_main_context_release(JMainContext * ctx);

/*
 * Tries to become the owner of the specified context, as with g_main_context_acquire().
 * But if another thread is the owner, atomically drop mutex and wait on cond until that owner releases ownership or until cond is signaled, then try again (once) to become the owner.
 */
jboolean j_main_context_wait(JMainContext * ctx, JCond * cond,
                             JMutex * mutex);

/*
 * Wakeup context if it is blocked
 */
void j_main_context_wakeup(JMainContext * ctx);

#endif
