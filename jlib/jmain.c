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
#include "jmain.h"
#include "jthread.h"
#include "jslist.h"
#include "jlist.h"
#include "jhashtable.h"
#include "jarray.h"
#include "jwakeup.h"
#include <time.h>


struct _JSourceFuncs {
    jboolean(*prepare) (JSource * source, jint * timeout);
    jboolean(*check) (JSource * source);
    jboolean(*dispatch) (JSource * source, JSourceFunc callback,
                         jpointer user_data);
    void (*finalize) (JSource * source);
};


struct _JSourceCallbackFuncs {
    void (*ref) (jpointer cb_data);
    void (*unref) (jpointer cb_data);
    void (*get) (jpointer cb_data, JSource * source,
                 JSourceFunc * func, jpointer * data);
};

struct _JSource {
    JSourceCallbackFuncs *callback_funcs;
    jpointer callback_data;

    /* 在不同阶段调用的JSource函数 */
    const JSourceFuncs *source_funcs;
    juint ref;

    JMainContext *context;

    jint priority;              /* 优先级 */
    juint flags;

    juint id;                   /* source id */

    JSList *poll_fds;

    jchar *name;

    JSList *children;
    JSource *parent;

    jint64 ready_time;
};

struct _JMainContext {
    JMutex mutex;
    JCond cond;
    JThread *owner;
    juint owner_count;
    JSList *waiters;

    jint ref;
    JHashTable *sources;        /* juint -> JSource */

    JPtrArray *pending_despatches;
    jint timeout;               /* timeout for current iteration */

    juint next_id;
    JList *source_lists;
    jint in_check_or_prepare;

    JWakeup *wakeup;

    jint64 time;
    jboolean time_is_fresh;
};



jint64 j_get_monotonic_time(void)
{
    struct timespec ts;
    jint result;

    result = clock_gettime(CLOCK_MONOTONIC, &ts);

    if (J_UNLIKELY(result != 0)) {
        return -1;
    }

    return (((jint64) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}
