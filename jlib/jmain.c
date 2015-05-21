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
#include "jslist.h"
#include "jlist.h"
#include "jhashtable.h"
#include "jarray.h"
#include "jwakeup.h"
#include "jmem.h"
#include "jatomic.h"
#include "jmessage.h"
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

    jint priority;              /* 优先级，当前没有用 */
    juint flags;

    juint id;                   /* source id */

    JSList *poll_fds;           /* JEPollEvent* */

    jchar *name;

    JSList *children;
    JSource *parent;

    jint64 ready_time;
};
#define J_SOURCE_IS_DESTROYED(src)  (((src)->flags & J_SOURCE_FLAG_ACTIVE) == 0)
#define J_SOURCE_IS_BLOCKED(src)    (((src)->flags & J_SOURCE_FLAG_BLOCKED) == 0)

#define J_SOURCE_UNREF(src, ctx)    J_STMT_START\
                                    if((src)->ref>1){\
                                        (src)->ref--;\
                                    }else{\
                                        j_source_unref_internal((src),(ctx),TRUE);\
                                    }J_STMT_END

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

    JEPoll *epoll;

    JPtrArray *poll_records;    /* JEPollEvent, fd就是文件描述符，events就是events，data是优先级 */
    JEPollEvent *cached_poll_array;
    juint cached_poll_array_size;
    jboolean poll_changed;

    jint64 time;
    jboolean time_is_fresh;
};

#define J_MAIN_CONTEXT_LOCK(ctx)    j_mutex_lock(&(ctx)->mutex)
#define J_MAIN_CONTEXT_UNLOCK(ctx)  j_mutex_unlock(&(ctx)->mutex)

#define J_MAIN_CONTEXT_CHECK(ctx)   J_STMT_START\
                                    if(ctx==NULL){ \
                                        ctx=j_main_context_default();\
                                    }J_STMT_END


typedef struct {
    JMutex *mutex;
    JCond *cond;
} JMainWaiter;


static jint compare_epoll_event(jconstpointer d1, jconstpointer d2)
{
    JEPollEvent *e1 = (JEPollEvent *) d1;
    JEPollEvent *e2 = (JEPollEvent *) d2;
    return e1->fd - e2->fd;
}

static inline void j_main_context_remove_poll_unlocked(JMainContext * ctx,
                                                       JEPollEvent * p)
{
    jint index =
        j_ptr_array_find_index(ctx->poll_records, compare_epoll_event, p);
    if (index >= 0) {
        j_ptr_array_remove_index_fast(ctx->poll_records, index);
    }
    if (j_epoll_del(ctx->epoll, p->fd, NULL)) {
        ctx->poll_changed = TRUE;
        j_wakeup_signal(ctx->wakeup);
    }
}

static inline void j_main_context_add_poll_unlocked(JMainContext * ctx,
                                                    jint priority,
                                                    JEPollEvent * p)
{
    //j_epoll_add(ctx->epoll, p->fd, p->events, p->data);
    JEPollEvent *new = j_new(JEPollEvent, 1);
    new->fd = p->fd;
    new->events = p->events | J_EPOLL_ERR | J_EPOLL_HUP;
    j_ptr_array_append_ptr(ctx->poll_records, new);
    ctx->poll_changed = TRUE;
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

juint j_source_get_id(JSource * src)
{
    if (J_UNLIKELY(src->context == NULL)) {
        return 0;
    }
    J_MAIN_CONTEXT_LOCK(src->context);
    juint result = src->id;
    J_MAIN_CONTEXT_UNLOCK(src->context);
    return result;
}

JMainContext *j_source_get_context(JSource * src)
{
    return src->context;
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
        J_MAIN_CONTEXT_LOCK(src->context);
        j_main_context_add_poll_unlocked(src->context, src->priority, e);
        J_MAIN_CONTEXT_UNLOCK(src->context);
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
        J_MAIN_CONTEXT_LOCK(ctx);
    }

    if (!J_SOURCE_IS_DESTROYED(src)) {
        JSList *tmp_list;
        jpointer old_cb_data;
        JSourceCallbackFuncs *old_cb_funcs;
        src->flags &= ~J_SOURCE_FLAG_ACTIVE;

        old_cb_data = src->callback_data;
        old_cb_funcs = src->callback_funcs;

        src->callback_data = NULL;
        src->callback_funcs = NULL;

        if (old_cb_funcs) {
            J_MAIN_CONTEXT_UNLOCK(ctx);
            old_cb_funcs->unref(old_cb_data);
            J_MAIN_CONTEXT_LOCK(ctx);
        }

        if (!J_SOURCE_IS_BLOCKED(src)) {
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
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }
}

/*
 * Holds context's lock
 */
static inline void source_add_to_context(JSource * src, JMainContext * ctx)
{
    ctx->source_lists = j_list_append(ctx->source_lists, src);
    j_hash_table_insert(ctx->sources, JUINT_TO_JPOINTER(src->id), src);
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
        J_MAIN_CONTEXT_LOCK(ctx);
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
                J_MAIN_CONTEXT_UNLOCK(ctx);
            }
            src->funcs->finalize(src);
            if (ctx) {
                J_MAIN_CONTEXT_LOCK(ctx);
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
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }

    if (old_cb_funcs) {
        if (has_lock) {
            J_MAIN_CONTEXT_UNLOCK(ctx);
        }
        old_cb_funcs->unref(old_cb_data);
        if (has_lock) {
            J_MAIN_CONTEXT_UNLOCK(ctx);
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
        J_MAIN_CONTEXT_LOCK(ctx);
    }
    src->ref++;
    if (ctx) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
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


J_LOCK_DEFINE_STATIC(default_context_lock);
static JMainContext *default_main_context = NULL;
/*
 * Returns the global default main context.
 */
JMainContext *j_main_context_default(void)
{
    J_LOCK(default_context_lock);
    if (J_UNLIKELY(default_main_context == NULL)) {
        default_main_context = j_main_context_new();
    }
    J_UNLOCK(default_context_lock);
    return default_main_context;
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
    ctx->poll_records = j_ptr_array_new_full(100, j_free);

    ctx->cached_poll_array = NULL;
    ctx->cached_poll_array_size = 0;

    ctx->pending_despatches = j_ptr_array_new();
    ctx->time_is_fresh = FALSE;

    ctx->wakeup = j_wakeup_new();
    JEPollEvent wakeup_event;
    j_wakeup_get_pollfd(ctx->wakeup, &wakeup_event);
    j_main_context_add_poll_unlocked(ctx, 0, &wakeup_event);

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

    J_MAIN_CONTEXT_LOCK(ctx);
    JList *tmp_list = ctx->source_lists;
    while (tmp_list) {
        JSource *src = (JSource *) j_list_data(tmp_list);
        src->context = NULL;
        j_source_destroy_internal(src, ctx, TRUE);
        tmp_list = j_list_next(tmp_list);
    }
    j_list_free(ctx->source_lists);

    j_ptr_array_free(ctx->poll_records, TRUE);
    J_MAIN_CONTEXT_UNLOCK(ctx);

    j_hash_table_free(ctx->sources);

    j_ptr_array_free(ctx->pending_despatches, TRUE);
    j_free(ctx->cached_poll_array);

    j_wakeup_free(ctx->wakeup);
    j_cond_clear(&ctx->cond);
    j_mutex_clear(&ctx->mutex);

    j_free(ctx);
}

/*
 * Tries to become the owner of the specified context.
 * If some other thread is the owner of the context, returns FALSE immediately.
 * Ownership is properly recursive: the owner can require ownership again and will release ownership when g_main_context_release() is called as many times as g_main_context_acquire().
 */
jboolean j_main_context_acquire(JMainContext * ctx)
{
    jboolean result = FALSE;
    JThread *self = j_thread_self();

    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    if (!ctx->owner) {
        ctx->owner = self;
    }
    if (ctx->owner == self) {
        ctx->owner_count++;
        result = TRUE;
    }
    J_MAIN_CONTEXT_UNLOCK(ctx);

    return result;
}


/*
 * Releases ownership of a context previously acquired by this thread with g_main_context_acquire().
 * If the context was acquired multiple times, the ownership will be released only when g_main_context_release() is called as many times as it was acquired.
 */
void j_main_context_release(JMainContext * ctx)
{
    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    ctx->owner_count--;
    if (ctx->owner_count == 0) {
        ctx->owner = NULL;
        if (ctx->waiters) {
            JMainWaiter *waiter = ctx->waiters->data;
            jboolean loop_internal_waiter = (waiter->mutex == &ctx->mutex);
            ctx->waiters = j_slist_delete_link(ctx->waiters, ctx->waiters);
            if (!loop_internal_waiter) {
                j_mutex_lock(waiter->mutex);
            }
            j_cond_signal(waiter->cond);
            if (!loop_internal_waiter) {
                j_mutex_unlock(waiter->mutex);
            }
        }
    }
    J_MAIN_CONTEXT_UNLOCK(ctx);
}

/*
 * Tries to become the owner of the specified context,
 * as with g_main_context_acquire().
 * But if another thread is the owner, atomically drop mutex and wait on
 * cond until that owner releases ownership or until cond is signaled,
 * then try again (once) to become the owner.
 */
jboolean j_main_context_wait(JMainContext * ctx, JCond * cond,
                             JMutex * mutex)
{
    jboolean result = FALSE;
    JThread *self = j_thread_self();
    jboolean loop_internal_waiter;

    J_MAIN_CONTEXT_CHECK(ctx);

    if (J_UNLIKELY(cond != &ctx->cond || mutex != &ctx->mutex)) {

    }

    loop_internal_waiter = (mutex == &ctx->mutex);

    if (!loop_internal_waiter) {
        J_MAIN_CONTEXT_LOCK(ctx);
    }

    if (ctx->owner && ctx->owner != self) {
        JMainWaiter waiter;
        waiter.cond = cond;
        waiter.mutex = mutex;

        ctx->waiters = j_slist_append(ctx->waiters, &waiter);

        if (!loop_internal_waiter) {
            J_MAIN_CONTEXT_UNLOCK(ctx);
        }
        j_cond_wait(cond, mutex);
        if (!loop_internal_waiter) {
            J_MAIN_CONTEXT_LOCK(ctx);
        }
        ctx->waiters = j_slist_remove(ctx->waiters, &waiter);
    }

    if (!ctx->owner) {
        ctx->owner = self;
    }
    if (ctx->owner == self) {
        ctx->owner_count++;
        result = TRUE;
    }

    if (!loop_internal_waiter) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }

    return result;
}

/*
 * Wakeup context if it is blocked
 */
void j_main_context_wakeup(JMainContext * ctx)
{
    J_MAIN_CONTEXT_CHECK(ctx);
    if (j_atomic_int_get(&ctx->ref) <= 0) {
        return;
    }
    j_wakeup_signal(ctx->wakeup);
}

static inline juint j_source_attach_unlocked(JSource * src,
                                             JMainContext * ctx,
                                             jboolean do_wakeup)
{
    JSList *tmp_list;
    juint id;

    do {
        id = ctx->next_id++;
    } while (J_UNLIKELY
             (id == 0
              || j_hash_table_contains(ctx->sources,
                                       JUINT_TO_JPOINTER(id))));

    src->context = ctx;
    src->id = id;
    src->ref++;

    source_add_to_context(src, ctx);

    if (!J_SOURCE_IS_BLOCKED(src)) {
        tmp_list = src->poll_fds;
        while (tmp_list) {
            j_main_context_add_poll_unlocked(ctx, src->priority,
                                             j_slist_data(tmp_list));
            tmp_list = j_slist_next(tmp_list);
        }
    }
    tmp_list = src->children;
    while (tmp_list) {
        j_source_attach_unlocked(j_slist_data(tmp_list), ctx, FALSE);
        tmp_list = j_slist_next(tmp_list);
    }

    /* If another thread has acquired the context, wake it up since
     * it might be in epoll_wait() right now
     */
    if (do_wakeup && ctx->owner && ctx->owner != j_thread_self()) {
        j_wakeup_signal(ctx->wakeup);
    }

    return src->id;
}

/*
 * Adds a GSource to a context so that it will be executed within that context.
 * Returns the source ID
 */
juint j_source_attach(JSource * src, JMainContext * ctx)
{
    if (src->context != NULL || J_SOURCE_IS_DESTROYED(src)) {
        return 0;
    }
    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    juint result = j_source_attach_unlocked(src, ctx, TRUE);
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return result;
}

jboolean j_main_context_prepare(JMainContext * ctx, jint * max_priority)
{
    juint i;
    jint n_ready = 0;
    jint current_priority = J_MAXINT32;

    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    ctx->time_is_fresh = FALSE; /* 是否已经刷新过时间  */
    if (ctx->in_check_or_prepare) {
        j_warning
            ("j_main_context_prepare() called recursively from within "
             "a source's check() or prepare() member");
        J_MAIN_CONTEXT_UNLOCK(ctx);
        return FALSE;
    }
    for (i = 0; i < ctx->pending_despatches->len; i += 1) {
        if (ctx->pending_despatches->data[i]) {
            J_SOURCE_UNREF((JSource *) ctx->pending_despatches->data[i],
                           ctx);
        }
    }
    j_ptr_array_set_size(ctx->pending_despatches, 0);

    /* Prepare all sources */
    ctx->timeout = -1;          /* -1 表示无限大 */
    JList *iter = ctx->source_lists;
    while (iter) {
        JSource *src = (JSource *) j_list_data(iter);
        jint source_timeout = -1;
        if (J_SOURCE_IS_DESTROYED(src) || J_SOURCE_IS_BLOCKED(src)) {
            goto CONTINUE;
        }
        if (n_ready > 0 && src->priority > current_priority) {
            /* 优先级相关，但现在没有使用优先级 */
            //break;
        }
        if (!(src->flags & J_SOURCE_FLAG_READY)) {
            /* 如果没有ready，则执行Source的prepare函数，看看会不会ready */
            jboolean result;
            jboolean(*prepare) (JSource * src, jint * timeout);
            prepare = src->funcs->prepare;
            if (prepare) {
                ctx->in_check_or_prepare++;
                J_MAIN_CONTEXT_UNLOCK(ctx);
                result = (*prepare) (src, &source_timeout);
                J_MAIN_CONTEXT_LOCK(ctx);
                ctx->in_check_or_prepare--;
            } else {            /* 没有prepare则默认会无限阻塞 */
                source_timeout = -1;
                result = FALSE;
            }

            if (result == FALSE && src->ready_time != -1) {
                /* 虽然阻塞，但设置了ready_time，则会在ready_time到时回调  */
                if (!ctx->time_is_fresh) {
                    ctx->time = j_get_monotonic_time();
                    ctx->time_is_fresh = TRUE;
                }
                if (src->ready_time <= ctx->time) {
                    /* ready_time的时间已过，就要立即回调 */
                    source_timeout = 0;
                    result = TRUE;
                } else {        /* 否则计算时间差来设置timeout */
                    jint timeout;
                    /* rounding down will lead to spinning, so always round up */
                    timeout = (src->ready_time - ctx->time + 999) / 1000;
                    if (source_timeout < 0 || timeout < source_timeout) {
                        source_timeout = timeout;
                    }
                }
            }
            if (result) {       /* 如果已经ready，那么将其父Source也设置为ready */
                JSource *ready_source = src;
                while (ready_source) {
                    ready_source->flags |= J_SOURCE_FLAG_READY;
                    ready_source = ready_source->parent;
                }
            }
        }
        if (src->flags & J_SOURCE_FLAG_READY) {
            /* 如果已经ready，那么context的timeout就是0 */
            n_ready++;
            current_priority = src->priority;
            ctx->timeout = 0;
        }
        if (source_timeout >= 0) {
            if (ctx->timeout < 0) {
                ctx->timeout = source_timeout;
            } else {
                ctx->timeout = MIN(ctx->timeout, source_timeout);
            }
        }
      CONTINUE:
        iter = j_list_next(iter);
    }

    J_MAIN_CONTEXT_UNLOCK(ctx);
    if (max_priority) {
        *max_priority = current_priority;
    }
    return n_ready > 0;
}


/*
 * Determines information necessary to poll this main loop.
 *
 * You must have successfully acquired the context with j_main_context_acquire()
 * before you call this function.
 *
 * Returns: the number of records actually stored in @fds,
 *   or, if more than @n_fds records need to be stored, the number
 *   of records that need to be stored
 */
jint j_main_context_query(JMainContext * ctx, jint max_priority,
                          jint * timeout)
{
    J_MAIN_CONTEXT_LOCK(ctx);
    jint i, total = j_ptr_array_get_len(ctx->poll_records);
    jpointer *data = j_ptr_array_get_data(ctx->poll_records);
    for (i = 0; i < total; i++) {
        JEPollEvent *event = (JEPollEvent *) data[i];
        if (!j_epoll_has(ctx->epoll, event->fd)) {
            j_epoll_add(ctx->epoll, event->fd, event->events, event->data);
        }
    }

    if (timeout) {
        *timeout = ctx->timeout;
        if (*timeout != 0) {
            ctx->time_is_fresh = FALSE;
        }
    }
    J_MAIN_CONTEXT_UNLOCK(ctx);

    return total;
}

jboolean j_main_context_check(JMainContext * ctx, jint max_priority,
                              JEPollEvent * fds, jint n_fds)
{
    /* TODO */
    return FALSE;
}

void j_main_context_dispatch(JMainContext * ctx)
{
    /* TODO */
}

static inline void j_main_context_poll(JMainContext * ctx, jint timeout,
                                       jint priority, JEPollEvent * fds,
                                       jint n_fds)
{
    /* TODO */
}

static inline jboolean j_main_context_iterate(JMainContext * ctx,
                                              jboolean may_block,
                                              jboolean dispatch,
                                              JThread * self)
{
    jint max_priority;
    jint timeout;
    jboolean some_ready;
    jint nfds, allocated_nfds;
    JEPollEvent *fds = NULL;

    J_MAIN_CONTEXT_UNLOCK(ctx);
    if (!j_main_context_acquire(ctx)) {
        jboolean got_ownership;
        J_MAIN_CONTEXT_LOCK(ctx);
        if (!may_block) {
            return FALSE;
        }
        got_ownership = j_main_context_wait(ctx, &ctx->cond, &ctx->mutex);
        if (!got_ownership) {
            return FALSE;
        }
    } else {
        J_MAIN_CONTEXT_LOCK(ctx);
    }

/*    if (ctx->cached_poll_array == NULL) {*/
/*        ctx->cached_poll_array_size = j_epoll_count(ctx->epoll) + 1;*/
/*        ctx->cached_poll_array =*/
/*            j_new(JEPollEvent, ctx->cached_poll_array_size);*/
/*    }*/

    allocated_nfds = ctx->cached_poll_array_size;
    fds = ctx->cached_poll_array;

    J_MAIN_CONTEXT_UNLOCK(ctx);

    j_main_context_prepare(ctx, &max_priority);

    if ((nfds = j_main_context_query(ctx, max_priority, &timeout)) >
        allocated_nfds) {
        J_MAIN_CONTEXT_LOCK(ctx);
        j_free(fds);
        ctx->cached_poll_array_size = allocated_nfds = nfds;
        ctx->cached_poll_array = fds = j_new(JEPollEvent, nfds);
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }

    if (!may_block) {
        timeout = 0;
    }

    j_main_context_poll(ctx, timeout, max_priority, fds, nfds);

    some_ready = j_main_context_check(ctx, max_priority, fds, nfds);

    if (dispatch) {
        j_main_context_dispatch(ctx);
    }

    j_main_context_release(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    return some_ready;
}

/*
 * Runs a single iteration
 */
jboolean j_main_context_iteration(JMainContext * ctx, jboolean may_block)
{
    jboolean retval;

    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    retval = j_main_context_iterate(ctx, may_block, TRUE, j_thread_self());
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return retval;
}
