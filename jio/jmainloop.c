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

#include "jmainloop.h"
#include "jpoll.h"
#include <jlib/jlib.h>


struct _JMainLoop {
    JPoll *poll;

    JHashTable *sources;

    int running;
};

static JMainLoop *main_loop = NULL;
/*
 * Gets the default main loop
 */
JMainLoop *j_main_loop_default(void)
{
    if (main_loop == NULL) {
        main_loop = j_main_loop_new();
    }
    return main_loop;
}

typedef enum {
    J_SOCKET_EVENT_ACCEPT,
    J_SOCKET_EVENT_SEND,
    J_SOCKET_EVENT_RECV,
} JSocketEventType;

typedef struct {
    JSocket *socket;
    JSocketEventType event;
    void *user_data;
    void *callback;

    void *data;
    unsigned int count;
    unsigned int len;
} JSource;

static inline JSource *j_source_new(JSocket * socket,
                                    JSocketEventType event, void *data,
                                    void *callback);
static inline JSocket *j_source_free(JSource * src);
#define j_source_get_socket(src)    (src)->socket
#define j_source_get_event(src)     (src)->event
#define j_source_get_user_data(src) (src)->user_data
#define j_source_get_callback(src)  (src)->callback
#define j_source_get_data(src)      (src)->data
#define j_source_get_data_count(src)    (src)->count
#define j_source_get_data_len(src)  (src)->len
static inline unsigned int j_source_get_events(JSource * src)
{
    if (j_source_get_event(src) == J_SOCKET_EVENT_SEND) {
        return J_POLLOUT;
    }
    return J_POLLIN;
}

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop);
static inline int j_main_loop_append_source(JMainLoop * loop,
                                            JSource * src);
static inline void j_main_loop_remove_source(JMainLoop * loop,
                                             JSource * src);
static inline int j_main_loop_modify_source(JMainLoop * loop,
                                            JSource * src);

/*
 * Allocates memory for JMainLoop structure
 */
static inline JMainLoop *j_main_loop_alloc(JPoll * poll);

/*
 * Creates a new main loop
 */
JMainLoop *j_main_loop_new(void)
{
    JPoll *poll = j_poll_new();
    if (poll == NULL) {
        return NULL;
    }
    return j_main_loop_alloc(poll);
}

/*
 * Runs the main loop
 */
void j_main_loop_run(JMainLoop * loop)
{
    loop->running = 1;
    JPoll *poll = j_main_loop_get_poll(loop);
    JPollEvent events[1024];
    int n;
    while ((n = j_poll_wait(poll, events, 1024, 100)) >= 0 && loop->running) {  /* 100 miliseconds */
        int i;
        for (i = 0; i < n; i++) {
            unsigned int evnts = events[i].events;
            JSource *src = (JSource *) events[i].data;
            JSocket *socket = j_source_get_socket(src);
            void *user_data = j_source_get_user_data(src);
            void *callback = j_source_get_callback(src);

            /* send or read */
            void *data = j_source_get_data(src);
            unsigned int count = j_source_get_data_count(src);
            unsigned int len = j_source_get_data_len(src);

            JSocketAcceptNotify accept_callback =
                (JSocketAcceptNotify) callback;
            JSocketSendNotify write_callback =
                (JSocketSendNotify) callback;
            JSocketRecvNotify read_callback = (JSocketRecvNotify) callback;
            switch (j_source_get_event(src)) {
            case J_SOCKET_EVENT_ACCEPT:
                if (evnts & J_POLLIN) {
                    JSocket *conn = j_socket_accept(socket);
                    if (!accept_callback(socket, conn, user_data)) {
                        j_main_loop_remove_source(loop, src);
                    }
                } else {
                    accept_callback(socket, NULL, user_data);
                    j_main_loop_remove_source(loop, src);
                }
                break;
            case J_SOCKET_EVENT_RECV:
                j_main_loop_remove_source(loop, src);
                if (evnts & J_POLLIN) {
                    JSocketRecvResult *res =
                        j_socket_recv_dontwait(socket, 0);
                    read_callback(socket, res, user_data);
                    j_socket_recv_result_free(res);
                } else {
                    read_callback(socket, NULL, user_data);
                }
                break;
            case J_SOCKET_EVENT_SEND:
                if (evnts & J_POLLOUT) {
                    int n = j_socket_send(socket, data + len, count - len);
                    if (n > 0) {
                        len += n;
                        if (len < count) {
                            src->len = len;
                            break;
                        }
                    }
                }
                src->data = NULL;
                j_main_loop_remove_source(loop, src);
                write_callback(socket, data, count, len, user_data);
                j_free(data);
                break;
            }
        }
    }
}

void j_main_loop_quit(JMainLoop * loop)
{
    loop->running = 0;
}

/*
 * Quits the default main loop
 */
void j_main_quit(void)
{
    JMainLoop *loop = j_main_loop_default();
    j_main_loop_quit(loop);
}

void j_main_loop_free(JMainLoop * loop)
{
    j_hash_table_free_full(loop->sources);
    j_poll_close(loop->poll);
    j_free(loop);
    if (loop == main_loop) {
        main_loop = NULL;
    }
}

/*
 * Runs the default main loop
 */
void j_main(void)
{
    JMainLoop *loop = j_main_loop_default();
    j_main_loop_run(loop);
}

void j_socket_accept_async(JSocket * sock, JSocketAcceptNotify notify,
                           void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src =
        j_source_new(sock, J_SOCKET_EVENT_ACCEPT, user_data, notify);
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, NULL, user_data);
    }
}

void j_socket_send_async(JSocket * sock, JSocketSendNotify notify,
                         const void *data, unsigned int count,
                         void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src =
        j_source_new(sock, J_SOCKET_EVENT_SEND, user_data, notify);
    src->data = j_memdup(data, count);
    src->count = count;
    src->len = 0;
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, data, count, 0, user_data);
    }
}

void j_socket_recv_async(JSocket * sock, JSocketRecvNotify notify,
                         void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src = j_source_new(sock, J_SOCKET_EVENT_RECV,
                                user_data, notify);
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, NULL, user_data);
    }
}


static inline JMainLoop *j_main_loop_alloc(JPoll * poll)
{
    JMainLoop *ml = (JMainLoop *) j_malloc(sizeof(JMainLoop));
    ml->poll = poll;
    ml->sources =
        j_hash_table_new(20, j_direct_hash, j_direct_equal,
                         NULL, (JValueDestroyFunc) j_source_free);
    return ml;
}

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop)
{
    return loop->poll;
}

#include <errno.h>
#include <string.h>
/*
 * Appends a new source
 * If the socket is already existing, modifies its events
 */
static inline int j_main_loop_append_source(JMainLoop * loop,
                                            JSource * src)
{
    JPoll *poll = j_main_loop_get_poll(loop);

    JPollOp op = J_POLL_CTL_ADD;
    if (j_hash_table_find(loop->sources, j_source_get_socket(src))) {
        op = J_POLL_CTL_MOD;
    }
    unsigned int events = j_source_get_events(src);
    if (!j_poll_ctl(poll, op, events, j_source_get_socket(src), src)) {
        j_source_free(src);
        return 0;
    }
    j_hash_table_insert(loop->sources, j_source_get_socket(src), src);
    return 1;
}

static inline void j_main_loop_remove_source(JMainLoop * loop,
                                             JSource * src)
{
    JPoll *poll = j_main_loop_get_poll(loop);
    j_poll_ctl(poll, J_POLL_CTL_DEL, J_POLLIN, j_source_get_socket(src),
               NULL);
    j_hash_table_remove_full(loop->sources, j_source_get_socket(src));
}

static inline int j_main_loop_modify_source(JMainLoop * loop,
                                            JSource * src)
{
    JPoll *poll = j_main_loop_get_poll(loop);

    unsigned int events = j_source_get_events(src);

    if (!j_poll_ctl(poll, J_POLL_CTL_MOD, events,
                    j_source_get_socket(src), src)) {
        j_source_free(src);
        return 0;
    }
    return 1;
}

static inline JSource *j_source_new(JSocket * socket,
                                    JSocketEventType event,
                                    void *user_data, void *callback)
{
    JSource *src = (JSource *) j_malloc(sizeof(JSource));
    src->socket = socket;
    src->event = event;
    src->user_data = user_data;
    src->callback = callback;
    src->data = NULL;
    src->count = 0;
    src->len = 0;
    return src;
}

static inline JSocket *j_source_free(JSource * src)
{
    JSocket *socket = src->socket;
    if (src->data) {
        j_free(src->data);
    }
    j_free(src);
    return socket;
}
