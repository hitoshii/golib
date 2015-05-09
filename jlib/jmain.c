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
#include "jatomic.h"
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

    JSList *poll_fds;           /* JEPollEvent* */

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
    JEPollEvent wakeup_event;

    JEPoll *epoll;

    JEPollEvent *cached_poll_array;
    juint cached_poll_array_size;
    jboolean poll_changed;

    jint64 time;
    jboolean time_is_fresh;
};

#define j_main_context_lock(ctx)    j_mutex_lock(&(ctx)->mutex)
#define j_main_context_unlock(ctx)  j_mutex_unlock(&(ctx)->mutex)


static inline void j_main_context_remove_poll_unlocked(JMainContext * ctx,
                                                       JEPollEvent * p)
{
    j_epoll_del(ctx->epoll, p->fd, NULL);
    j_wakeup_signal(ctx->wakeup);
}

static inline void j_main_context_add_poll_unlocked(JMainContext * ctx,
                                                    JEPollEvent * p)
{
    j_epoll_add(ctx->epoll, p->fd, p->events, p->data);
    j_wakeup_signal(ctx->wakeup);
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

/*
 * Monitors fd for the IO events in events .
 */
void j_source_add_poll_fd(JSource * src, jint fd, juint io)
{
    JEPollEvent *e = (JEPollEvent *) j_malloc(sizeof(JEPollEvent));
    e->fd = fd;
    e->events = io;
    e->data = src;
    src->poll_fds = j_slist_append(src->poll_fds, e);
    if (src->context) {
        j_main_context_lock(src->context);
        j_main_context_add_poll_unlocked(src->context, e);
        j_main_context_unlock(src->context);
    }
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

/*
 * Holds context's lock
 */
static inline void source_remove_from_context(JSource * src,
                                              JMainContext * ctx)
{
    ctx->source_lists = j_list_remove(ctx->source_lists, src);
    j_hash_table_remove(ctx->sources, JUINT_TO_JPOINTER(src->id));
}

/*
 * j_source_unref() but possible to call within context lock
 */
static inline void j_source_unref_internal(JSource * src,
                                           JMainContext * ctx,
                                           jboolean has_lock)
{
    jpointer old_cb_data = NULL;
    JSourceCallbackFuncs *old_cb_funcs = NULL;
    if (J_UNLIKELY(src == NULL)) {
        return;
    }

    if (!has_lock && ctx) {
        j_main_context_lock(ctx);
    }

    src->ref--;
    if (src->ref == 0) {
        old_cb_data = src->callback_data;
        old_cb_funcs = src->callback_funcs;

        src->callback_data = NULL;
        src->callback_funcs = NULL;

        if (ctx) {
            source_remove_from_context(src, ctx);
        }

        if (src->funcs->finalize) {
            if (ctx) {
                j_main_context_unlock(ctx);
            }
            src->funcs->finalize(src);
            if (ctx) {
                j_main_context_lock(ctx);
            }
        }

        j_free(src->name);
        src->name = NULL;

        j_slist_free(src->poll_fds);
        src->poll_fds = NULL;

        j_slist_free_full(src->poll_fds, j_free);
        JSList *tmp_list = src->children;
        while (tmp_list) {
            JSource *child = (JSource *) j_slist_data(tmp_list);
            j_source_unref_internal(child, ctx, has_lock);
            tmp_list = j_slist_next(tmp_list);
        }
        j_slist_free(src->children);
        src->children = NULL;

        j_free(src);
    }

    if (!has_lock && ctx) {
        j_main_context_unlock(ctx);
    }

    if (old_cb_funcs) {
        if (has_lock) {
            j_main_context_unlock(ctx);
        }
        old_cb_funcs->unref(old_cb_data);
        if (has_lock) {
            j_main_context_unlock(ctx);
        }
    }
}

/*
 * Increases the reference count on a source by one.
 */
void j_source_ref(JSource * src)
{
    JMainContext *ctx = src->context;
    if (ctx) {
        j_main_context_lock(ctx);
    }
    src->ref++;
    if (ctx) {
        j_main_context_unlock(ctx);
    }
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



/*
 * Creates a new JMainContext
 */
JMainContext *j_main_context_new(void)
{
    JMainContext *ctx = (JMainContext *) j_new0(JMainContext, 1);

    j_mutex_init(&ctx->mutex);
    j_cond_init(&ctx->cond);

    ctx->sources =
        j_hash_table_new(16, j_int_hash, j_int_equal, NULL, NULL);
    ctx->owner = NULL;
    ctx->waiters = NULL;

    ctx->ref = 1;

    ctx->next_id = 1;

    ctx->source_lists = NULL;

    ctx->epoll = j_epoll_new();

    ctx->cached_poll_array = NULL;
    ctx->cached_poll_array_size = 0;

    ctx->pending_despatches = j_ptr_array_new();
    ctx->time_is_fresh = FALSE;

    ctx->wakeup = j_wakeup_new();
    j_wakeup_get_pollfd(ctx->wakeup, &ctx->wakeup_event);
    j_main_context_add_poll_unlocked(ctx, &ctx->wakeup_event);

    return ctx;
}

/*
 * Increases the reference count on a context by one
 */
void j_main_context_ref(JMainContext * ctx)
{
    if (J_UNLIKELY(j_atomic_int_get(&ctx->ref) <= 0)) {
        return;
    }
    j_atomic_int_inc(&ctx->ref);
}

/*
 * Decreases the reference
 */
void j_main_context_unref(JMainContext * ctx)
{
    if (J_UNLIKELY(j_atomic_int_get(&ctx->ref) <= 0)) {
        return;
    }

    if (!j_atomic_int_dec_and_test(&ctx->ref)) {
        return;
    }

    jint i;
    /* Free pending dispatches */
    for (i = 0; i < ctx->pending_despatches->len; i++) {
        j_source_unref_internal(ctx->pending_despatches->data[i], ctx,
                                FALSE);
    }

    j_main_context_lock(ctx);
    JList *tmp_list = ctx->source_lists;
    while (tmp_list) {
        JSource *src = (JSource *) j_list_data(tmp_list);
        src->context = NULL;
        j_source_destroy_internal(src, ctx, TRUE);
        tmp_list = j_list_next(tmp_list);
    }
    j_list_free(ctx->source_lists);
    j_main_context_unlock(ctx);

    j_hash_table_free(ctx->sources);

    j_ptr_array_free(ctx->pending_despatches, TRUE);
    j_free(ctx->cached_poll_array);

    j_wakeup_free(ctx->wakeup);
    j_cond_clear(&ctx->cond);
    j_mutex_clear(&ctx->mutex);

    j_free(ctx);
}
