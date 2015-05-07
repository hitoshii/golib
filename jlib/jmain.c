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
#include "jmem.h"
#include "jepoll.h"
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
    const JSourceFuncs *funcs;
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
#define j_source_is_destroyed(src)  (((src)->flags & J_SOURCE_FLAG_ACTIVE) == 0)
#define j_source_is_blocked(src)    (((src)->flags & J_SOURCE_FLAG_BLOCKED) == 0)

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

#define j_main_context_lock(ctx)    j_mutex_lock(&(ctx)->mutex)
#define j_main_context_unlock(ctx)  j_mutex_unlock(&(ctx)->mutex)


static inline void j_main_context_remove_poll_unlocked(JMainContext * ctx,
                                                       JEPollEvent * p)
{
    /* TODO */
}

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


/*
 * Creates a new JSource structure.
 * The size is specified to allow createing structures derived from JSource that contain additional data.
 * The size must be greater than sizeof(JSource)
 */
JSource *j_source_new(JSourceFuncs * funcs, juint struct_size)
{
    if (J_UNLIKELY(struct_size < sizeof(JSource))) {
        return NULL;
    }
    JSource *src = (JSource *) j_malloc0(struct_size);
    src->funcs = funcs;
    src->ref = 1;
    src->ready_time = -1;
    src->flags = J_SOURCE_FLAG_ACTIVE;
    src->priority = J_PRIORITY_DEFAULT;
    return src;
}


const jchar *j_source_get_name(JSource * src)
{
    return src->name;
}

static inline void j_source_unref_internal(JSource * src,
                                           JMainContext * ctx,
                                           jboolean has_lock);
static inline void j_source_destroy_internal(JSource * src,
                                             JMainContext * ctx,
                                             jboolean has_lock);

static inline void j_source_destroy_internal(JSource * src,
                                             JMainContext * ctx,
                                             jboolean has_lock)
{
    if (!has_lock) {
        j_main_context_lock(ctx);
    }

    if (!j_source_is_destroyed(src)) {
        JSList *tmp_list;
        jpointer old_cb_data;
        JSourceCallbackFuncs *old_cb_funcs;
        src->flags &= ~J_SOURCE_FLAG_ACTIVE;

        old_cb_data = src->callback_data;
        old_cb_funcs = src->callback_funcs;

        src->callback_data = NULL;
        src->callback_funcs = NULL;

        if (old_cb_funcs) {
            j_main_context_unlock(ctx);
            old_cb_funcs->unref(old_cb_data);
            j_main_context_lock(ctx);
        }

        if (!j_source_is_blocked(src)) {
            tmp_list = src->poll_fds;
            while (tmp_list) {
                j_main_context_remove_poll_unlocked(ctx,
                                                    j_slist_data
                                                    (tmp_list));
                tmp_list = j_slist_next(tmp_list);
            }
        }

        tmp_list = src->children;
        while (tmp_list) {
            JSource *child = (JSource *) j_slist_data(tmp_list);
            j_source_destroy_internal(child, ctx, TRUE);
            j_source_unref_internal(child, ctx, TRUE);
            tmp_list = j_slist_next(tmp_list);
        }
        if (src->parent) {
            src->parent->children =
                j_slist_remove(src->parent->children, src);
        }

        j_source_unref_internal(src, ctx, TRUE);
    }
    if (!has_lock) {
        j_main_context_unlock(ctx);
    }
}

static inline void j_source_unref_internal(JSource * src,
                                           JMainContext * ctx,
                                           jboolean has_lock)
{
    /* TODO */
}

void j_source_unref(JSource * src)
{
    j_source_unref_internal(src, src->context, FALSE);
}

/*
 * Removes a source from its GMainContext, if any, and mark it as destroyed.
 * The source cannot be subsequently added to another context.
 * It is safe to call this on sources which have already been removed from their context.
 */
void j_source_destroy(JSource * src)
{
    JMainContext *context = src->context;
    if (context == NULL) {
        src->flags &= ~J_SOURCE_FLAG_ACTIVE;
    } else {
        j_source_destroy_internal(src, context, FALSE);
    }
}
