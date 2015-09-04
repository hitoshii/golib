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
#ifndef __JLIB_MAIN_H__
#define __JLIB_MAIN_H__
#include "jtypes.h"
#include "jxpoll.h"
#include "jthread.h"
#include "jslist.h"

/*
 * Queries the system monotonic time.
 */
int64_t j_get_monotonic_time(void);

/* JSource */
typedef boolean(*JSourceFunc) (void * user_data);

typedef struct _JSourceCallbackFuncs JSourceCallbackFuncs;
typedef struct _JSourceFuncs JSourceFuncs;
typedef struct _JSource JSource;

/* JMainContext */
typedef struct _JMainContext JMainContext;


/* JMainLoop */
typedef struct _JMainLoop JMainLoop;


typedef enum {
    J_SOURCE_FLAG_ACTIVE = 1 << 0,
    J_SOURCE_FLAG_IN_CALL = 1 << 1,
    J_SOURCE_FLAG_BLOCKED = 1 << 2,
    J_SOURCE_FLAG_READY = 1 << 3,
    J_SOURCE_FLAG_CAN_RECURSE = 1 << 4,
    J_SOURCE_FLAG_MASK = 0xFF,
} JSourceFlag;

/*
 * JSource的事件函数
 */
struct _JSourceFuncs {
    boolean(*prepare) (JSource * source, int * timeout);
    boolean(*check) (JSource * source);
    boolean(*dispatch) (JSource * source, JSourceFunc callback,
                        void * user_data);
    void (*finalize) (JSource * source);
};

struct _JSource {
    JSourceCallbackFuncs *callback_funcs;
    void * callback_data;

    /* 在不同阶段调用的JSource函数 */
    const JSourceFuncs *funcs;
    unsigned int ref;

    JMainContext *context;
    unsigned int flags;
    unsigned int id;                   /* source id */
    JSList *poll_fds;           /* JXPollRecord* */
    char *name;

    int64_t ready_time;
};


const char *j_source_get_name(JSource * src);
unsigned int j_source_get_id(JSource * src);
JMainContext *j_source_get_context(JSource * src);
void j_source_set_ready_time(JSource * src, int64_t ready_time);
void j_source_set_callback(JSource * src, JSourceFunc func,
                           void * data, JDestroyNotify destroy);
void j_source_set_callback_indirect(JSource * src, void * callback_data,
                                    JSourceCallbackFuncs * callback_funcs);

/*
 * Creates a new JSource structure.
 * The size is specified to allow createing structures derived from JSource that contain additional data.
 * The size must be greater than sizeof(JSource)
 */
JSource *j_source_new(JSourceFuncs * funcs, unsigned int struct_size);

/*
 * Monitors fd for the IO events in events .
 */
void j_source_add_poll_fd(JSource * src, int fd, unsigned int io);


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

boolean j_source_is_destroyed(JSource * src);


int64_t j_source_get_time(JSource * src);


/*
 * Adds a GSource to a context so that it will be executed within that context.
 * Returns the source ID
 */
unsigned int j_source_attach(JSource * src, JMainContext * ctx);

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
boolean j_main_context_acquire(JMainContext * ctx);


/*
 * Releases ownership of a context previously acquired by this thread with g_main_context_acquire().
 * If the context was acquired multiple times, the ownership will be released only when g_main_context_release() is called as many times as it was acquired.
 */
void j_main_context_release(JMainContext * ctx);


/*
 * 检查当前线程是否拥有了该JMainContext
 */
boolean j_main_context_is_owner(JMainContext * ctx);

/*
 * Tries to become the owner of the specified context, as with g_main_context_acquire().
 * But if another thread is the owner, atomically drop mutex and wait on cond until that owner releases ownership or until cond is signaled, then try again (once) to become the owner.
 */
boolean j_main_context_wait(JMainContext * ctx, JCond * cond,
                            JMutex * mutex);

/*
 * Wakeup context if it is blocked
 */
void j_main_context_wakeup(JMainContext * ctx);

boolean j_main_context_prepare(JMainContext * ctx);
int j_main_context_query(JMainContext * ctx, int * timeout);
boolean j_main_context_check(JMainContext * ctx,
                             JXPollEvent * fds, int n_fds);
void j_main_context_dispatch(JMainContext * ctx);

/*
 * Runs a single iteration
 */
boolean j_main_context_iteration(JMainContext * ctx, boolean may_block);

int j_main_depth(void);
JSource *j_main_current_source(void);

/*
 * Creates a new JMainLoop
 */
JMainLoop *j_main_loop_new(JMainContext * ctx, boolean is_running);

boolean j_main_loop_is_running(JMainLoop * loop);
JMainContext *j_main_loop_get_context(JMainLoop * loop);

void j_main_loop_ref(JMainLoop * loop);
void j_main_loop_unref(JMainLoop * loop);

/*
 * Runs a main loop until j_main_loop_quit() is called on this loop
 */
void j_main_loop_run(JMainLoop * loop);
/*
 * Stops loop from running
 */
void j_main_loop_quit(JMainLoop * loop);

void j_main(void);
void j_main_quit(void);


/*
 * 定时回调
 */
typedef struct _JTimeoutSource JTimeoutSource;

JSource *j_timeout_source_new(unsigned int interval);

unsigned int j_timeout_add_full(uint32_t interval,
                                JSourceFunc function, void * data,
                                JDestroyNotify destroy);
unsigned int j_timeout_add(uint32_t interval, JSourceFunc function, void * data);

/*
 * XXX seconds版本用来减少回调的次数，它并不保证回调及时，可能快一点也可能慢一点
 */
JSource *j_timeout_source_new_seconds(unsigned int interval);
unsigned int j_timeout_add_seconds_full(unsigned int interval,
                                        JSourceFunc function, void * data,
                                        JDestroyNotify destroy);
unsigned int j_timeout_add_seconds(uint32_t interval, JSourceFunc function,
                                   void * data);

/*
 * 空闲回调
 */
JSource *j_idle_source_new(void);
unsigned int j_idle_add_full(JSourceFunc function, void * data,
                             JDestroyNotify destroy);
unsigned int j_idle_add(JSourceFunc function, void * data);


#endif
