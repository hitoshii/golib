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
#include "jmain.h"
#include "jenviron.h"
#include "jlist.h"
#include "jhashtable.h"
#include "jarray.h"
#include "jwakeup.h"
#include "jmem.h"
#include "jatomic.h"
#include "jmessage.h"
#include <time.h>


typedef struct {
    JXPollEvent event;
    unsigned short revent;             /* epoll_wait 返回的事件 */
} JXPollRecord;


struct _JSourceCallbackFuncs {
    void (*ref) (void * cb_data);
    void (*unref) (void * cb_data);
    void (*get) (void * cb_data, JSource * source,
                 JSourceFunc * func, void * * data);
};


#define J_SOURCE_IS_DESTROYED(src)  (((src)->flags & J_SOURCE_FLAG_ACTIVE) == 0)
#define J_SOURCE_IS_BLOCKED(src)    (((src)->flags & J_SOURCE_FLAG_BLOCKED))

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
    unsigned int owner_count;
    JSList *waiters;

    int ref;
    JHashTable *sources;        /* unsigned int -> JSource */

    JPtrArray *pending_dispatches;
    int timeout;               /* timeout for current iteration */

    unsigned int next_id;
    JList *source_lists;
    int in_check_or_prepare;

    JWakeup *wakeup;
    JXPollRecord wakeup_record;

    JXPoll *xp;

    JPtrArray *poll_records;
    JXPollEvent *cached_poll_array;
    unsigned int cached_poll_array_size;
    boolean poll_changed;

    int64_t time;
    boolean time_is_fresh;
};

#define J_MAIN_CONTEXT_LOCK(ctx)    j_mutex_lock(&(ctx)->mutex)
#define J_MAIN_CONTEXT_UNLOCK(ctx)  j_mutex_unlock(&(ctx)->mutex)
#define J_MAIN_CONTEXT_TRYLOCK(ctx) j_mutex_trylock(&(ctx)->mutex)

#define J_MAIN_CONTEXT_CHECK(ctx)   J_STMT_START\
                                    if(ctx==NULL){ \
                                        ctx=j_main_context_default();\
                                    }J_STMT_END


struct _JMainLoop {
    JMainContext *context;
    boolean is_running;
    int ref;
};


typedef struct {
    unsigned int ref;
    JSourceFunc func;
    void * data;
    JDestroyNotify notify;
} JSourceCallback;

typedef struct {
    JMutex *mutex;
    JCond *cond;
} JMainWaiter;


typedef struct {
    int depth;
    JSource *source;
} JMainDispatch;

static inline void j_main_dispatch_free(JMainDispatch * dispatch) {
    j_free(dispatch);
}

static inline JMainDispatch *j_get_dispatch(void) {
    J_PRIVATE_DEFINE_STATIC(depth_private, j_main_dispatch_free);
    JMainDispatch *dispatch = j_private_get(&depth_private);

    if (!dispatch) {
        dispatch = j_malloc0(sizeof(JMainDispatch));
        j_private_set(&depth_private, dispatch);
    }
    return dispatch;
}

int j_main_depth(void) {
    JMainDispatch *dispatch = j_get_dispatch();
    return dispatch->depth;
}

JSource *j_main_current_source(void) {
    JMainDispatch *dispatch = j_get_dispatch();
    return dispatch->source;
}

boolean j_source_is_destroyed(JSource * src) {
    return J_SOURCE_IS_DESTROYED(src);
}

static inline void j_main_context_remove_poll_unlocked(JMainContext * ctx,
        JXPollEvent * p) {
    j_ptr_array_remove(ctx->poll_records, p);
    if (j_xpoll_del(ctx->xp, p->fd, p->events)) {
        ctx->poll_changed = TRUE;
        j_wakeup_signal(ctx->wakeup);
    }
}


static inline void j_main_context_add_poll_unlocked(JMainContext * ctx,
        JXPollEvent * p) {
    j_ptr_array_append_ptr(ctx->poll_records, p);
    ctx->poll_changed = TRUE;
    j_wakeup_signal(ctx->wakeup);
}

int64_t j_get_monotonic_time(void) {
    struct timespec ts;
    int result;

    result = clock_gettime(CLOCK_MONOTONIC, &ts);

    j_return_val_if_fail(result == 0, -1);

    return (((int64_t) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}


/*
 * Creates a new JSource structure.
 * The size is specified to allow createing structures derived from JSource that contain additional data.
 * The size must be greater than sizeof(JSource)
 */
JSource *j_source_new(JSourceFuncs * funcs, unsigned int struct_size) {
    j_return_val_if_fail(struct_size >= sizeof(JSource), NULL);

    JSource *src = (JSource *) j_malloc0(struct_size);
    src->funcs = funcs;
    src->ref = 1;
    src->ready_time = -1;
    src->flags = J_SOURCE_FLAG_ACTIVE;
    return src;
}


const char *j_source_get_name(JSource * src) {
    return src->name;
}

unsigned int j_source_get_id(JSource * src) {
    j_return_val_if_fail(src->context != NULL, 0);
    J_MAIN_CONTEXT_LOCK(src->context);
    unsigned int result = src->id;
    J_MAIN_CONTEXT_UNLOCK(src->context);
    return result;
}

JMainContext *j_source_get_context(JSource * src) {
    j_return_val_if_fail(src != NULL, NULL);
    return src->context;
}


int64_t j_source_get_time(JSource * src) {
    j_return_val_if_fail(src->context != NULL, 0);

    JMainContext *ctx = src->context;

    J_MAIN_CONTEXT_LOCK(ctx);
    if (!ctx->time_is_fresh) {
        ctx->time = j_get_monotonic_time();
        ctx->time_is_fresh = TRUE;
    }
    int64_t result = ctx->time;
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return result;
}

void j_source_set_ready_time(JSource * src, int64_t ready_time) {
    j_return_if_fail(src != NULL && src->ref > 0);
    if (src->ready_time == ready_time) {
        return;
    }

    JMainContext *ctx = src->context;

    if (ctx) {
        J_MAIN_CONTEXT_LOCK(ctx);
    }

    src->ready_time = ready_time;

    if (ctx) {
        if (!J_SOURCE_IS_BLOCKED(src)) {
            j_wakeup_signal(ctx->wakeup);
        }
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }
}

void j_source_set_callback_indirect(JSource * src, void * callback_data,
                                    JSourceCallbackFuncs * callback_funcs) {
    j_return_if_fail(src != NULL && callback_funcs != NULL
                     && callback_data != NULL);

    JMainContext *ctx = src->context;
    if (ctx) {
        J_MAIN_CONTEXT_LOCK(ctx);
    }
    void * old_cb_data = src->callback_data;
    JSourceCallbackFuncs *old_cb_funcs = src->callback_funcs;

    src->callback_data = callback_data;
    src->callback_funcs = callback_funcs;

    if (ctx) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }
    if (old_cb_funcs) {
        old_cb_funcs->unref(old_cb_data);
    }
}

static void j_source_callback_ref(void * cb_data) {
    JSourceCallback *callback = cb_data;
    callback->ref++;
}

static void j_source_callback_unref(void * cb_data) {
    JSourceCallback *callback = cb_data;
    callback->ref--;
    if (callback->ref == 0) {
        if (callback->notify) {
            callback->notify(callback->data);
        }
        j_free(callback);
    }
}

static void j_source_callback_get(void * cb_data,
                                  JSource * src, JSourceFunc * func,
                                  void * * data) {
    JSourceCallback *callback = (JSourceCallback *) cb_data;
    *func = callback->func;
    *data = callback->data;
}

static JSourceCallbackFuncs j_source_callback_funcs = {
    j_source_callback_ref,
    j_source_callback_unref,
    j_source_callback_get,
};

void j_source_set_callback(JSource * src, JSourceFunc func,
                           void * data, JDestroyNotify destroy) {
    j_return_if_fail(src != NULL);

    JSourceCallback *new_callback = j_malloc(sizeof(JSourceCallback));
    new_callback->ref = 1;
    new_callback->func = func;
    new_callback->data = data;
    new_callback->notify = destroy;

    j_source_set_callback_indirect(src, new_callback,
                                   &j_source_callback_funcs);
}

/*
 * Monitors fd for the IO events in events .
 */
void j_source_add_poll_fd(JSource * src, int fd, unsigned int io) {
    JXPollRecord *e = (JXPollRecord *) j_malloc(sizeof(JXPollRecord));
    e->event.fd = fd;
    e->event.events = io;
    e->event.user_data = e;
    e->revent = 0;
    src->poll_fds = j_slist_append(src->poll_fds, e);
    if (src->context) {
        J_MAIN_CONTEXT_LOCK(src->context);
        j_main_context_add_poll_unlocked(src->context, &e->event);
        J_MAIN_CONTEXT_UNLOCK(src->context);
    }
}

static inline void j_source_unref_internal(JSource * src,
        JMainContext * ctx,
        boolean has_lock);
static inline void j_source_destroy_internal(JSource * src,
        JMainContext * ctx,
        boolean has_lock);

static inline void j_source_destroy_internal(JSource * src,
        JMainContext * ctx,
        boolean has_lock) {
    if (!has_lock) {
        J_MAIN_CONTEXT_LOCK(ctx);
    }

    if (!J_SOURCE_IS_DESTROYED(src)) {
        JSList *tmp_list;
        void * old_cb_data;
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
                JXPollRecord *rec =
                    (JXPollRecord *) j_slist_data(tmp_list);
                j_main_context_remove_poll_unlocked(ctx, &rec->event);
                tmp_list = j_slist_next(tmp_list);
            }
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
static inline void source_add_to_context(JSource * src, JMainContext * ctx) {
    ctx->source_lists = j_list_append(ctx->source_lists, src);
    j_hash_table_insert(ctx->sources, JUINT_TO_JPOINTER(src->id), src);
}

/*
 * Holds context's lock
 */
static inline void source_remove_from_context(JSource * src,
        JMainContext * ctx) {
    ctx->source_lists = j_list_remove(ctx->source_lists, src);
    j_hash_table_remove(ctx->sources, JUINT_TO_JPOINTER(src->id));
}

/*
 * j_source_unref() but possible to call within context lock
 */
static inline void j_source_unref_internal(JSource * src,
        JMainContext * ctx,
        boolean has_lock) {
    j_return_if_fail(src != NULL);

    void * old_cb_data = NULL;
    JSourceCallbackFuncs *old_cb_funcs = NULL;

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

        j_slist_free_full(src->poll_fds, j_free);

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
void j_source_ref(JSource * src) {
    JMainContext *ctx = src->context;
    if (ctx) {
        J_MAIN_CONTEXT_LOCK(ctx);
    }
    src->ref++;
    if (ctx) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
    }
}

void j_source_unref(JSource * src) {
    j_source_unref_internal(src, src->context, FALSE);
}

/*
 * Removes a source from its GMainContext, if any, and mark it as destroyed.
 * The source cannot be subsequently added to another context.
 * It is safe to call this on sources which have already been removed from their context.
 */
void j_source_destroy(JSource * src) {
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
JMainContext *j_main_context_default(void) {
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
JMainContext *j_main_context_new(void) {
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

    ctx->xp = j_xpoll_new();
    ctx->poll_records = j_ptr_array_new_full(100, NULL);

    ctx->cached_poll_array = NULL;
    ctx->cached_poll_array_size = 0;

    ctx->pending_dispatches = j_ptr_array_new();
    ctx->time_is_fresh = FALSE;

    ctx->wakeup = j_wakeup_new();
    j_wakeup_get_pollfd(ctx->wakeup, &(ctx->wakeup_record.event));
    ctx->wakeup_record.event.user_data = &(ctx->wakeup_record);
    j_main_context_add_poll_unlocked(ctx, &(ctx->wakeup_record.event));

    return ctx;
}

/*
 * Increases the reference count on a context by one
 */
void j_main_context_ref(JMainContext * ctx) {
    j_return_if_fail(j_atomic_int_get(&ctx->ref) > 0);

    j_atomic_int_inc(&ctx->ref);
}

/*
 * Decreases the reference
 */
void j_main_context_unref(JMainContext * ctx) {
    j_return_if_fail(j_atomic_int_get(&ctx->ref) > 0);

    if (!j_atomic_int_dec_and_test(&ctx->ref)) {
        return;
    }

    int i;
    /* Free pending dispatches */
    for (i = 0; i < ctx->pending_dispatches->len; i++) {
        j_source_unref_internal(ctx->pending_dispatches->data[i], ctx,
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

    j_ptr_array_free(ctx->pending_dispatches, TRUE);
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
boolean j_main_context_acquire(JMainContext * ctx) {
    boolean result = FALSE;
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
void j_main_context_release(JMainContext * ctx) {
    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    ctx->owner_count--;
    if (ctx->owner_count == 0) {
        ctx->owner = NULL;
        if (ctx->waiters) {
            JMainWaiter *waiter = ctx->waiters->data;
            boolean loop_internal_waiter = (waiter->mutex == &ctx->mutex);
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
 * 检查当前线程是否拥有了该JMainContext
 */
boolean j_main_context_is_owner(JMainContext * ctx) {
    if (ctx == NULL) {
        ctx = j_main_context_default();
    }
    J_MAIN_CONTEXT_LOCK(ctx);
    boolean is_owner = ctx->owner == j_thread_self();
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return is_owner;
}

/*
 * Tries to become the owner of the specified context,
 * as with g_main_context_acquire().
 * But if another thread is the owner, atomically drop mutex and wait on
 * cond until that owner releases ownership or until cond is signaled,
 * then try again (once) to become the owner.
 */
boolean j_main_context_wait(JMainContext * ctx, JCond * cond,
                            JMutex * mutex) {
    boolean result = FALSE;
    JThread *self = j_thread_self();
    boolean loop_internal_waiter;

    J_MAIN_CONTEXT_CHECK(ctx);

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
void j_main_context_wakeup(JMainContext * ctx) {
    J_MAIN_CONTEXT_CHECK(ctx);
    if (j_atomic_int_get(&ctx->ref) <= 0) {
        return;
    }
    j_wakeup_signal(ctx->wakeup);
}

static inline unsigned int j_source_attach_unlocked(JSource * src,
        JMainContext * ctx,
        boolean do_wakeup) {
    JSList *tmp_list;
    unsigned int id;

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
            JXPollRecord *rec = (JXPollRecord *) j_slist_data(tmp_list);
            j_main_context_add_poll_unlocked(ctx, &rec->event);
            tmp_list = j_slist_next(tmp_list);
        }
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
unsigned int j_source_attach(JSource * src, JMainContext * ctx) {
    if (src->context != NULL || J_SOURCE_IS_DESTROYED(src)) {
        return 0;
    }
    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    unsigned int result = j_source_attach_unlocked(src, ctx, TRUE);
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return result;
}

boolean j_main_context_prepare(JMainContext * ctx) {
    unsigned int i;
    int n_ready = 0;

    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    ctx->time_is_fresh = FALSE; /* 是否已经刷新过时间  */
    if (ctx->in_check_or_prepare) {
        /* j_main_context_prepare() called recursively from within a source's check() or prepare() member */;
        J_MAIN_CONTEXT_UNLOCK(ctx);
        return FALSE;
    }
    for (i = 0; i < ctx->pending_dispatches->len; i += 1) {
        if (ctx->pending_dispatches->data[i]) {
            J_SOURCE_UNREF((JSource *) ctx->pending_dispatches->data[i],
                           ctx);
        }
    }
    j_ptr_array_set_size(ctx->pending_dispatches, 0);

    /* Prepare all sources */
    ctx->timeout = -1;          /* -1 表示无限大 */
    JList *iter = ctx->source_lists;
    while (iter) {
        JSource *src = (JSource *) j_list_data(iter);
        int source_timeout = -1;
        if (J_SOURCE_IS_DESTROYED(src) || J_SOURCE_IS_BLOCKED(src)) {
            goto CONTINUE;
        }
        if (!(src->flags & J_SOURCE_FLAG_READY)) {
            /* 如果没有ready，则执行Source的prepare函数，看看会不会ready */
            boolean result;
            boolean(*prepare) (JSource * src, int * timeout);
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
                    int timeout;
                    /* rounding down will lead to spinning, so always round up */
                    timeout = (src->ready_time - ctx->time + 999) / 1000;
                    if (source_timeout < 0 || timeout < source_timeout) {
                        source_timeout = timeout;
                    }
                }
            }
            if (result) {
                src->flags |= J_SOURCE_FLAG_READY;
            }
        }
        if (src->flags & J_SOURCE_FLAG_READY) {
            /* 如果已经ready，那么context的timeout就是0 */
            n_ready++;
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
    return n_ready > 0;
}


/*
 * 保证所有文件描述符号都已经再EPoll中注册
 *
 * You must have successfully acquired the context with j_main_context_acquire()
 * before you call this function.
 *
 * Returns: the number of records actually stored in @fds,
 *   or, if more than @n_fds records need to be stored, the number
 *   of records that need to be stored
 */
int j_main_context_query(JMainContext * ctx, int * timeout) {
    J_MAIN_CONTEXT_LOCK(ctx);
    int i, total = j_ptr_array_get_len(ctx->poll_records);
    void * *data = j_ptr_array_get_data(ctx->poll_records);
    for (i = 0; i < total; i++) {
        JXPollEvent *event = (JXPollEvent *) data[i];
        JXPollRecord *rec = (JXPollRecord *) event->user_data;
        rec->revent = 0;        /* clear */
        j_xpoll_add(ctx->xp, event->fd, event->events, event->user_data);
    }
    ctx->wakeup_record.revent = 0;
    ctx->poll_changed = FALSE;

    if (timeout) {
        *timeout = ctx->timeout;
        if (*timeout != 0) {
            ctx->time_is_fresh = FALSE;
        }
    }

    if (ctx->cached_poll_array_size < total) {
        j_free(ctx->cached_poll_array);
        ctx->cached_poll_array_size = total;
        ctx->cached_poll_array = j_new(JXPollEvent, total);
    }

    J_MAIN_CONTEXT_UNLOCK(ctx);

    return MAX(total, j_xpoll_event_count(ctx->xp));
}

boolean j_main_context_check(JMainContext * ctx,
                             JXPollEvent * fds, int n_fds) {
    J_MAIN_CONTEXT_LOCK(ctx);
    if (ctx->in_check_or_prepare) {
        /* j_main_context_check() called recursively from "
                  "within a source's check() or prepare() member */
        J_MAIN_CONTEXT_UNLOCK(ctx);
        return FALSE;
    }
    if (ctx->wakeup_record.revent) {
        j_wakeup_acknowledge(ctx->wakeup);
    }


    int n_ready = 0;

    /*
     * If the set of poll file descriptors changed, bail out
     * and let the main loop rerun
     * XXX
     */
    if (ctx->poll_changed) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
        return FALSE;
    }

    JList *tmp_list = ctx->source_lists;
    while (tmp_list) {
        JSource *src = (JSource *) j_list_data(tmp_list);
        if (J_SOURCE_IS_DESTROYED(src) || J_SOURCE_IS_BLOCKED(src)) {
            goto CONTINUE;
        }
        if (!(src->flags & J_SOURCE_FLAG_READY)) {
            boolean result;
            boolean(*check) (JSource * src);
            check = src->funcs->check;
            if (check) {
                /* If the check function is set, call it */
                ctx->in_check_or_prepare++;
                J_MAIN_CONTEXT_UNLOCK(ctx);
                result = (*check) (src);
                J_MAIN_CONTEXT_LOCK(ctx);
                ctx->in_check_or_prepare--;
            } else {
                result = FALSE;
            }
            if (result == FALSE) {
                JSList *tmp_slist = src->poll_fds;
                while (tmp_slist) {
                    JXPollRecord *rec =
                        (JXPollRecord *) j_slist_data(tmp_slist);
                    if (rec->revent) {
                        result = TRUE;
                        break;
                    }
                    tmp_slist = j_slist_next(tmp_slist);
                }
            }
            if (result == FALSE && src->ready_time != -1) {
                if (!ctx->time_is_fresh) {
                    ctx->time = j_get_monotonic_time();
                    ctx->time_is_fresh = TRUE;
                }
                if (src->ready_time <= ctx->time) {
                    result = TRUE;
                }
            }
            if (result) {
                src->flags |= J_SOURCE_FLAG_READY;
            }
        }
        if (src->flags & J_SOURCE_FLAG_READY) {
            src->ref++;
            j_ptr_array_append_ptr(ctx->pending_dispatches, src);
            n_ready++;
        }
CONTINUE:
        tmp_list = j_list_next(tmp_list);
    }
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return n_ready > 0;
}

/*
 * Temporarily remove all this source's file descriptors from the context,
 * so that if data comes avaiable for one of the file descriptors
 * we don't continually spin in the epoll_wait()
 */
static inline void j_source_block(JSource * src) {
    j_return_if_fail(!J_SOURCE_IS_BLOCKED(src));    /* already blocked */

    JSList *tmp_list;
    src->flags |= J_SOURCE_FLAG_BLOCKED;

    if (src->context) {
        tmp_list = src->poll_fds;
        while (tmp_list) {
            JXPollRecord *rec = (JXPollRecord *) j_slist_data(tmp_list);
            j_main_context_remove_poll_unlocked(src->context,
                                                &(rec->event));
            tmp_list = j_slist_next(tmp_list);
        }
    }
}

static inline void j_source_unblock(JSource * src) {
    j_return_if_fail(J_SOURCE_IS_BLOCKED(src)
                     && !J_SOURCE_IS_DESTROYED(src));

    JSList *tmp_list;
    src->flags &= ~J_SOURCE_FLAG_BLOCKED;
    tmp_list = src->poll_fds;
    while (tmp_list) {
        JXPollRecord *rec = (JXPollRecord *) j_slist_data(tmp_list);
        j_main_context_add_poll_unlocked(src->context, &(rec->event));
        tmp_list = j_slist_next(tmp_list);
    }
}


/*
 * j_main_context_dispatch
 *
 * Dispatches all pending sources
 *
 * You must have successfully acquired the context with
 * j_main_context_acquire() before you may call this function
 */
void j_main_context_dispatch(JMainContext * ctx) {
    J_MAIN_CONTEXT_LOCK(ctx);

    JMainDispatch *current = j_get_dispatch();

    if (j_ptr_array_get_len(ctx->pending_dispatches) == 0) {
        J_MAIN_CONTEXT_UNLOCK(ctx);
        return;
    }
    unsigned int i;
    for (i = 0; i < j_ptr_array_get_len(ctx->pending_dispatches); i += 1) {
        JSource *src = ctx->pending_dispatches->data[i];
        ctx->pending_dispatches->data[i] = NULL;
        src->flags &= ~J_SOURCE_FLAG_READY;
        if (!J_SOURCE_IS_DESTROYED(src)) {
            boolean was_in_call;
            void * user_data = NULL;
            JSourceFunc callback = NULL;
            JSourceCallbackFuncs *cb_funcs;
            void * cb_data;
            boolean need_destroy;
            boolean(*dispatch) (JSource * src, JSourceFunc, void *);
            JSource *prev_src;

            dispatch = src->funcs->dispatch;
            cb_funcs = src->callback_funcs;
            cb_data = src->callback_data;

            if (cb_funcs) {
                cb_funcs->ref(cb_data);
            }
            if ((src->flags & J_SOURCE_FLAG_CAN_RECURSE) == 0) {
                j_source_block(src);
            }
            was_in_call = src->flags & J_SOURCE_FLAG_IN_CALL;
            src->flags |= J_SOURCE_FLAG_IN_CALL;

            if (cb_funcs) {
                cb_funcs->get(cb_data, src, &callback, &user_data);
            }
            J_MAIN_CONTEXT_UNLOCK(ctx);

            /* These operations are safe because 'current' is thread-local
             * and not modified from anywhere but this function
             */
            prev_src = current->source;
            current->source = src;
            current->depth++;

            need_destroy = !(*dispatch) (src, callback, user_data);

            current->source = prev_src;
            current->depth--;

            if (cb_funcs) {
                cb_funcs->unref(cb_data);
            }

            J_MAIN_CONTEXT_LOCK(ctx);

            if (!was_in_call) {
                src->flags &= ~J_SOURCE_FLAG_IN_CALL;
            }

            if (J_SOURCE_IS_BLOCKED(src) && !J_SOURCE_IS_DESTROYED(src)) {
                j_source_unblock(src);
            }

            /* Note: this depends on the fact that we can't switch
             * sources from one main context to another
             */
            if (need_destroy && !J_SOURCE_IS_DESTROYED(src)) {
                j_source_destroy_internal(src, ctx, TRUE);
            }
        }
        /* JSource在添加到pending_despatches中时增加了引用计数 */
        J_SOURCE_UNREF(src, ctx);
    }

    /* 清楚所有Source */
    j_ptr_array_set_size(ctx->pending_dispatches, 0);
    J_MAIN_CONTEXT_UNLOCK(ctx);
}

/*
 * Hold context lock
 */
static inline int j_main_context_poll(JMainContext * ctx, int timeout,
                                      JXPollEvent * fds, int n_fds) {
    int i, n;
    J_MAIN_CONTEXT_LOCK(ctx);
    n = j_xpoll_wait(ctx->xp, fds, n_fds, timeout);
    for (i = 0; i < n; i += 1) {
        /* 这里设置相应JSource的poll_records */
        JXPollRecord *rec = (JXPollRecord *) (fds[i].user_data);
        rec->revent |= fds[i].events;
    }
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return n;
}

/*
 * @param self unused
 */
static inline boolean j_main_context_iterate(JMainContext * ctx,
        boolean may_block,
        boolean dispatch,
        JThread * self) {
    int timeout;
    boolean some_ready;
    int nfds;
    JXPollEvent *fds = NULL;

    J_MAIN_CONTEXT_UNLOCK(ctx);
    if (!j_main_context_acquire(ctx)) {
        boolean got_ownership;
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

    J_MAIN_CONTEXT_UNLOCK(ctx);

    j_main_context_prepare(ctx);

    j_main_context_query(ctx, &timeout);
    nfds = ctx->cached_poll_array_size;
    fds = ctx->cached_poll_array;

    if (!may_block) {
        timeout = 0;
    }

    nfds = j_main_context_poll(ctx, timeout, fds, nfds);

    some_ready = j_main_context_check(ctx, fds, nfds);

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
boolean j_main_context_iteration(JMainContext * ctx, boolean may_block) {
    boolean retval;

    J_MAIN_CONTEXT_CHECK(ctx);
    J_MAIN_CONTEXT_LOCK(ctx);
    retval = j_main_context_iterate(ctx, may_block, TRUE, j_thread_self());
    J_MAIN_CONTEXT_UNLOCK(ctx);
    return retval;
}


JMainLoop *j_main_loop_new(JMainContext * ctx, boolean is_running) {
    if (ctx == NULL) {
        ctx = j_main_context_default();
    }
    j_main_context_ref(ctx);

    JMainLoop *loop = j_malloc0(sizeof(JMainLoop));
    loop->context = ctx;
    loop->is_running = is_running != FALSE;
    loop->ref = 1;
    return loop;
}

boolean j_main_loop_is_running(JMainLoop * loop) {
    j_return_val_if_fail(loop != NULL
                         && j_atomic_int_get(&loop->ref) > 0, FALSE);
    return loop->is_running;
}

JMainContext *j_main_loop_get_context(JMainLoop * loop) {
    j_return_val_if_fail(loop != NULL
                         && j_atomic_int_get(&loop->ref) > 0, NULL);

    return loop->context;
}

void j_main_loop_ref(JMainLoop * loop) {
    j_return_if_fail(loop != NULL && j_atomic_int_get(&loop->ref) > 0);

    j_atomic_int_inc(&loop->ref);
}

void j_main_loop_unref(JMainLoop * loop) {
    j_return_if_fail(loop != NULL && j_atomic_int_get(&loop->ref) > 0);

    if (!j_atomic_int_dec_and_test(&loop->ref)) {
        return;
    }
    j_main_context_unref(loop->context);
    j_free(loop);
}

/*
 * Runs a main loop until j_main_loop_quit() is called on this loop
 */
void j_main_loop_run(JMainLoop * loop) {
    j_return_if_fail(loop != NULL && j_atomic_int_get(&loop->ref) > 0);

    JThread *self = j_thread_self();

    if (!j_main_context_acquire(loop->context)) {
        boolean got_ownership = FALSE;
        J_MAIN_CONTEXT_LOCK(loop->context);

        j_atomic_int_inc(&loop->ref);
        if (!loop->is_running) {
            loop->is_running = TRUE;
        }

        while (loop->is_running && !got_ownership) {
            got_ownership = j_main_context_wait(loop->context,
                                                &loop->context->cond,
                                                &loop->context->mutex);
        }

        if (!loop->is_running) {
            J_MAIN_CONTEXT_UNLOCK(loop->context);
            if (got_ownership) {
                j_main_context_release(loop->context);
            }
            j_main_loop_unref(loop);
            return;
        }
    } else {
        J_MAIN_CONTEXT_LOCK(loop->context);
    }

    if (loop->context->in_check_or_prepare) {
        /* j_main_loop_run() called recursively from within a
           source's check() or prepare() member, iteration not possible. */
        return;
    }

    j_atomic_int_inc(&loop->ref);
    loop->is_running = TRUE;
    while (loop->is_running) {
        j_main_context_iterate(loop->context, TRUE, TRUE, self);
    }

    J_MAIN_CONTEXT_UNLOCK(loop->context);
    j_main_context_release(loop->context);

    j_main_loop_unref(loop);
}

/*
 * Stops loop from running
 */
void j_main_loop_quit(JMainLoop * loop) {
    j_return_if_fail(loop != NULL && j_atomic_int_get(&loop->ref) > 0);

    J_MAIN_CONTEXT_LOCK(loop->context);
    loop->is_running = FALSE;
    j_wakeup_signal(loop->context->wakeup);
    j_cond_broadcast(&loop->context->cond);
    J_MAIN_CONTEXT_UNLOCK(loop->context);
}

/*
 * 试图获取JMainContext的锁，如果成功，相当于调用了j_main_quit()
 * 否则只是将loop->is_running设置为FALSE
 */
void j_main_loop_try_quit(JMainLoop *loop) {
    j_return_if_fail(loop != NULL && j_atomic_int_get(&loop->ref) > 0);

    JMainContext *ctx=loop->context;
    loop->is_running = FALSE;
    if(J_MAIN_CONTEXT_TRYLOCK(ctx)) {
        j_wakeup_signal(loop->context->wakeup);
        j_cond_broadcast(&loop->context->cond);
        J_MAIN_CONTEXT_UNLOCK(loop->context);
    }
}


static JMainLoop *default_main_loop = NULL;
J_MUTEX_DEFINE_STATIC(default_main_loop_mutex);

void j_main(void) {
    j_mutex_lock(&default_main_loop_mutex);
    if (default_main_loop == NULL) {
        default_main_loop = j_main_loop_new(NULL, FALSE);
    }
    j_mutex_unlock(&default_main_loop_mutex);
    j_main_loop_run(default_main_loop);
}

void j_main_quit(void) {
    j_mutex_lock(&default_main_loop_mutex);
    if (default_main_loop) {
        j_main_loop_quit(default_main_loop);
    }
    j_mutex_unlock(&default_main_loop_mutex);
}

/*
 * 试图获取JMainContext的锁，如果成功，相当于调用了j_main_quit()
 * 否则只是将loop->is_running设置为FALSE
 */
void j_main_try_quit(void) {
    j_mutex_lock(&default_main_loop_mutex);
    if (default_main_loop) {
        j_main_loop_try_quit(default_main_loop);
    }
    j_mutex_unlock(&default_main_loop_mutex);
}

/***************************** TIMEOUT ****************************/

struct _JTimeoutSource {
    JSource source;
    unsigned int interval;
    boolean seconds;
};

static boolean j_timeout_dispatch(JSource * src, JSourceFunc callback,
                                  void * user_data);
static void j_timeout_set_expiration(JTimeoutSource * src,
                                     int64_t current_time);

JSourceFuncs j_timeout_funcs = {
    NULL,                       /* prepare */
    NULL,                       /* check */
    j_timeout_dispatch,
    NULL,
};

static boolean j_timeout_dispatch(JSource * src, JSourceFunc callback,
                                  void * user_data) {
    JTimeoutSource *timeout_src = (JTimeoutSource *) src;
    boolean again;
    if (J_UNLIKELY(!callback)) {
        /* Timeout source dispatched without callback You must call j_source_set_callback() */
        return FALSE;
    }
    again = callback(user_data);
    if (again) {
        j_timeout_set_expiration(timeout_src, j_source_get_time(src));
    }
    return again;
}

/*
 * 设置超时时间
 */
static void j_timeout_set_expiration(JTimeoutSource * src,
                                     int64_t current_time) {
    int64_t expiration = current_time + (uint64_t) src->interval * 1000;

    if (src->seconds) {
        static int timer_perturb = -1;

        if (timer_perturb == -1) {
            /*
             * We want a per machine/session unique 'random' value;
             * use the hostname for hashing
             */
            const char *hostname = j_getenv("HOSTNAME");
            if (hostname) {
                timer_perturb = ABS((int) j_str_hash(hostname)) % 1000000;
            } else {
                timer_perturb = 0;
            }
        }
        /* We want the microseconds part of the timeout to land on the
         * 'timer_perturb' mark, but we need to make sure we don't try to
         * set the timeout in the past. We do this by ensuring that we
         * always only *increase* the expiration time by adding a full
         * second in the case that the microsecond portion descreses.
         */
        expiration -= timer_perturb;
        int64_t remainder = expiration % 1000000;
        if (remainder >= 1000000 / 4) {
            expiration += 1000000;
        }
        expiration -= remainder;
        expiration += timer_perturb;
    }
    j_source_set_ready_time((JSource *) src, expiration);
}

JSource *j_timeout_source_new(unsigned int interval) {
    JSource *src = j_source_new(&j_timeout_funcs, sizeof(JTimeoutSource));
    JTimeoutSource *timeout_src = (JTimeoutSource *) src;

    timeout_src->interval = interval;
    j_timeout_set_expiration(timeout_src, j_get_monotonic_time());

    return src;
}


unsigned int j_timeout_add_full(uint32_t interval,
                                JSourceFunc function, void * data,
                                JDestroyNotify destroy) {
    j_return_val_if_fail(function != NULL, 0);

    JSource *src = j_timeout_source_new(interval);

    j_source_set_callback(src, function, data, destroy);
    unsigned int id = j_source_attach(src, NULL);
    j_source_unref(src);

    return id;
}

JSource *j_timeout_source_new_seconds(unsigned int interval) {
    JSource *src = j_source_new(&j_timeout_funcs, sizeof(JTimeoutSource));
    JTimeoutSource *timeout_src = (JTimeoutSource *) src;

    timeout_src->interval = 1000 * interval;
    timeout_src->seconds = TRUE;

    j_timeout_set_expiration(timeout_src, j_get_monotonic_time());

    return src;
}

unsigned int j_timeout_add(uint32_t interval, JSourceFunc function, void * data) {
    return j_timeout_add_full(interval, function, data, NULL);
}

unsigned int j_timeout_add_seconds_full(unsigned int interval,
                                        JSourceFunc function, void * data,
                                        JDestroyNotify destroy) {
    j_return_val_if_fail(function != NULL, 0);

    JSource *src = j_timeout_source_new_seconds(interval);
    j_source_set_callback(src, function, data, destroy);
    unsigned int id = j_source_attach(src, NULL);
    j_source_unref(src);
    return id;
}

unsigned int j_timeout_add_seconds(uint32_t interval, JSourceFunc function,
                                   void * data) {
    return j_timeout_add_seconds_full(interval, function, data, NULL);
}

/********************************* IDLE ***************************/

static boolean j_idle_prepare(JSource * src, int * timeout);
static boolean j_idle_check(JSource * src);
static boolean j_idle_dispatch(JSource * src, JSourceFunc callback,
                               void * user_data);

JSourceFuncs j_idle_funcs = {
    j_idle_prepare,
    j_idle_check,
    j_idle_dispatch,
    NULL,
};


static boolean j_idle_prepare(JSource * src, int * timeout) {
    *timeout = 0;
    return TRUE;
}

static boolean j_idle_check(JSource * src) {
    return TRUE;
}

static boolean j_idle_dispatch(JSource * src, JSourceFunc callback,
                               void * user_data) {
    return callback(user_data);
}

JSource *j_idle_source_new(void) {
    JSource *src = j_source_new(&j_idle_funcs, sizeof(JSource));
    return src;
}


unsigned int j_idle_add_full(JSourceFunc function, void * data,
                             JDestroyNotify destroy) {
    j_return_val_if_fail(function != NULL, 0);

    JSource *src = j_idle_source_new();

    j_source_set_callback(src, function, data, destroy);
    unsigned int id = j_source_attach(src, NULL);
    j_source_unref(src);
    return id;
}

unsigned int j_idle_add(JSourceFunc function, void * data) {
    return j_idle_add_full(function, data, NULL);
}
