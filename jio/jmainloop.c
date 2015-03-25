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
    J_SOCKET_EVENT_SEND_RECV,
    J_SOCKET_EVENT_KEEP,
} JSocketEventType;

typedef struct {
    JSocket *socket;
    JSocketEventType event;
    void *user_data;
    /* callback */
    JSocketAcceptNotify accept_callback;
    JSocketRecvNotify recv_callback;
    JSocketSendNotify send_callback;

    /* send */
    JByteArray *send_data;
    unsigned int send_len;
    /* recv */
    unsigned int recv_len;
    JByteArray *recv_data;
} JSource;
#define j_source_get_socket(src)    (src)->socket
#define j_source_get_event(src)     (src)->event
#define j_source_get_user_data(src) (src)->user_data
#define j_source_get_accept_callback(src)  (src)->accept_callback
#define j_source_get_recv_callback(src)     (src)->recv_callback
#define j_source_get_send_callback(src)     (src)->send_callback
#define j_source_get_send_data(src)      (src)->send_data
#define j_source_get_send_len(src)  (src)->send_len
#define j_source_get_recv_data(src) (src)->recv_data
#define j_source_get_recv_len(src)  (src)->recv_len

static inline JSource *j_source_new(JSocket * socket,
                                    JSocketEventType event, void *data,
                                    void *callback);
static inline JSocket *j_source_free(JSource * src);


static inline unsigned int j_source_get_events(JSource * src)
{
    if (j_source_get_event(src) == J_SOCKET_EVENT_SEND) {
        return J_POLLOUT;
    } else if (j_source_get_event(src) == J_SOCKET_EVENT_RECV) {
        return J_POLLIN;
    } else if (j_source_get_event(src) == J_SOCKET_EVENT_SEND_RECV) {
        return J_POLLIN | J_POLLOUT;
    } else if (j_source_get_event(src) == J_SOCKET_EVENT_ACCEPT) {
        return J_POLLIN;
    }
    return J_POLLERR;
}

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop);
static inline int j_main_loop_append_source(JMainLoop * loop,
                                            JSource * src);
static inline void j_main_loop_remove_source(JMainLoop * loop,
                                             JSource * src);
static inline void j_main_loop_update_source(JMainLoop * loop,
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
            JSocketAcceptNotify accept_callback =
                j_source_get_accept_callback(src);
            JSocketSendNotify send_callback =
                j_source_get_send_callback(src);
            JSocketRecvNotify read_callback =
                j_source_get_recv_callback(src);
            /* send */
            JByteArray *send_bytes = j_source_get_send_data(src);
            void *send_data =
                send_bytes ? j_byte_array_get_data(send_bytes) : NULL;
            unsigned int send_count =
                send_bytes ? j_byte_array_get_len(send_bytes) : 0;
            unsigned int send_len = j_source_get_send_len(src);
            /* recv */
            unsigned int recv_len = j_source_get_recv_len(src);
            JByteArray *recv_data = j_source_get_recv_data(src);


            switch (j_source_get_event(src)) {
            case J_SOCKET_EVENT_ACCEPT:    /* Accept */
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
            case J_SOCKET_EVENT_KEEP:  /* Keep */
            case J_SOCKET_EVENT_SEND:
            case J_SOCKET_EVENT_RECV:
            case J_SOCKET_EVENT_SEND_RECV:
                if (evnts & J_POLLOUT) {
                    int n = j_socket_send_dontwait(socket,
                                                   send_data + send_len,
                                                   send_count - send_len);
                    if (n > 0) {
                        send_len += n;
                        if (send_len < send_count) {
                            src->send_len = send_len;
                        } else {
                            src->send_data = NULL;
                            if (src->event == J_SOCKET_EVENT_SEND_RECV) {
                                src->event = J_SOCKET_EVENT_RECV;
                                j_main_loop_update_source(loop, src);
                            } else {
                                j_main_loop_remove_source(loop, src);
                            }
                            send_callback(socket, send_data, send_count,
                                          send_len, user_data);
                            j_byte_array_free(send_bytes, 1);
                        }
                    }
                }
                if (evnts & J_POLLIN) {
                    JSocketRecvResult *res =
                        j_socket_recv_dontwait(socket, recv_len);
                    j_byte_array_append(recv_data,
                                        j_socket_recv_result_get_data(res),
                                        j_socket_recv_result_get_len(res));
                    if (j_socket_recv_result_get_len(res) < recv_len
                        && j_socket_recv_result_is_normal(res)) {
                        src->recv_len -= j_socket_recv_result_get_len(res);
                    } else {
                        src->recv_data = NULL;
                        if (src->event == J_SOCKET_EVENT_SEND_RECV) {
                            src->event = J_SOCKET_EVENT_SEND;
                            j_main_loop_update_source(loop, src);
                        } else {
                            j_main_loop_remove_source(loop, src);
                        }
                        read_callback(socket,
                                      j_byte_array_get_data(recv_data),
                                      j_byte_array_get_len(recv_data),
                                      j_socket_recv_result_get_type(res),
                                      user_data);
                        j_byte_array_free(recv_data, 1);
                    }
                    j_socket_recv_result_free(res);
                }
                if (evnts & J_POLLHUP || evnts & J_POLLERR) {
                    src->send_data = NULL;
                    j_main_loop_remove_source(loop, src);
                    if (read_callback) {
                        read_callback(socket, NULL, 0, J_SOCKET_RECV_ERR,
                                      user_data);
                    }
                    if (send_callback) {
                        send_callback(socket, send_data, send_count,
                                      send_len, user_data);
                    }
                    j_byte_array_free(send_data, 1);
                }
                break;
            }
        }
    }

    j_main_loop_free(loop);
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
    src->send_data = j_byte_array_new();
    j_byte_array_append(src->send_data, data, count);
    src->send_len = 0;
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, data, count, 0, user_data);
    }
}

void j_socket_recv_async(JSocket * sock, JSocketRecvNotify notify,
                         void *user_data)
{
    j_socket_recv_len_async(sock, notify, 0, user_data);
}

void j_socket_recv_len_async(JSocket * sock, JSocketRecvNotify notify,
                             unsigned int len, void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src = j_source_new(sock, J_SOCKET_EVENT_RECV,
                                user_data, notify);
    src->recv_len = len;
    src->recv_data = j_byte_array_new();
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, NULL, 0, J_SOCKET_RECV_ERR, user_data);
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

/*
 * Appends a new source
 * If the socket is already existing, modifies its events
 */
static inline int j_main_loop_append_source(JMainLoop * loop,
                                            JSource * src)
{
    JPoll *poll = j_main_loop_get_poll(loop);
    JSocket *sock = j_source_get_socket(src);
    JSource *origin = j_hash_table_find(loop->sources, sock);

    if (origin == NULL) {
        unsigned int events = j_source_get_events(src);
        if (!j_poll_ctl(poll, J_POLL_CTL_ADD, events, sock, src)) {
            j_source_free(src);
            return 0;
        }
        j_hash_table_insert(loop->sources, sock, src);
        return 1;
    }

    unsigned int origin_event = j_source_get_event(origin);
    unsigned int event = j_source_get_event(src);

    if (origin_event == J_SOCKET_EVENT_ACCEPT
        || event == J_SOCKET_EVENT_ACCEPT) {
        j_source_free(src);
        return 0;
    } else if (event == J_SOCKET_EVENT_KEEP) {
        j_source_free(src);
        return 1;
    } else if (origin_event == J_SOCKET_EVENT_KEEP) {
        j_main_loop_remove_source(loop, origin);
        return j_main_loop_append_source(loop, src);
    } else if (origin_event == event) {
        if (event == J_SOCKET_EVENT_RECV) {
            j_source_free(src);
            return 1;
        } else if (event == J_SOCKET_EVENT_SEND) {
            j_byte_array_append(origin->send_data,
                                j_byte_array_get_data(src->send_data),
                                j_byte_array_get_len(src->send_data));
            j_source_free(src);
            return 1;
        }
    } else if (origin_event == J_SOCKET_EVENT_RECV) {
        origin->event = J_SOCKET_EVENT_SEND_RECV;
        origin->send_data = src->send_data;
        origin->send_len = src->send_len;
        origin->send_callback = src->send_callback;
        src->send_data = NULL;
        j_source_free(src);

        unsigned int events = j_source_get_events(origin);
        if (!j_poll_ctl(poll, J_POLL_CTL_MOD, events, sock, origin)) {
            j_main_loop_remove_source(loop, origin);
            return 0;
        }
    } else if (origin_event == J_SOCKET_EVENT_SEND) {
        origin->event = J_SOCKET_EVENT_SEND_RECV;
        origin->recv_data = src->recv_data;
        origin->recv_len = src->recv_len;
        origin->recv_callback = src->recv_callback;
        src->recv_data = NULL;
        j_source_free(src);

        unsigned int events = j_source_get_events(origin);
        if (!j_poll_ctl(poll, J_POLL_CTL_MOD, events, sock, origin)) {
            j_main_loop_remove_source(loop, origin);
            return 0;
        }
    } else if (origin_event == J_SOCKET_EVENT_SEND_RECV) {
        if (event == J_SOCKET_EVENT_RECV) {
            j_source_free(src);
            return 1;
        }
        j_byte_array_append(origin->send_data,
                            j_byte_array_get_data(src->send_data),
                            j_byte_array_get_len(src->send_data));
        j_source_free(src);
        return 1;
    }
    return 1;
}

/*
 * 更新poll监听事件
 */
static inline void j_main_loop_update_source(JMainLoop * loop,
                                             JSource * src)
{
    JSocket *sock = j_source_get_socket(src);
    JPoll *poll = j_main_loop_get_poll(loop);

    unsigned int events = j_source_get_events(src);
    if (!j_poll_ctl(poll, J_POLL_CTL_MOD, events, sock, src)) {
        j_main_loop_remove_source(loop, src);
    }
}

static inline void j_main_loop_remove_source(JMainLoop * loop,
                                             JSource * src)
{
    JPoll *poll = j_main_loop_get_poll(loop);
    j_poll_ctl(poll, J_POLL_CTL_DEL, J_POLLIN, j_source_get_socket(src),
               NULL);
    j_hash_table_remove_full(loop->sources, j_source_get_socket(src));
}

static inline JSource *j_source_new(JSocket * socket,
                                    JSocketEventType event,
                                    void *user_data, void *callback)
{
    JSource *src = (JSource *) j_malloc(sizeof(JSource));
    src->socket = socket;
    src->event = event;
    src->user_data = user_data;
    src->accept_callback = NULL;
    src->send_callback = NULL;
    src->recv_callback = NULL;
    src->send_data = NULL;
    src->send_len = 0;
    src->recv_data = NULL;
    src->recv_len = 0;
    if (event == J_SOCKET_EVENT_ACCEPT) {
        src->accept_callback = callback;
    } else if (event == J_SOCKET_EVENT_RECV) {
        src->recv_callback = callback;
    } else if (event == J_SOCKET_EVENT_SEND) {
        src->send_callback = callback;
    }
    return src;
}

static inline JSocket *j_source_free(JSource * src)
{
    JSocket *socket = src->socket;
    if (src->recv_data) {
        j_byte_array_free(src->recv_data, 1);
    }
    if (src->send_data) {
        j_byte_array_free(src->send_data, 1);
    }
    j_free(src);
    return socket;
}
