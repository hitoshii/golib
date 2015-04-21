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

typedef struct {
    JSocket *socket;
    unsigned int events;
    int accept;
    /* callback */
    JSocketAcceptNotify accept_callback;
    JSocketRecvNotify recv_callback;
    JSocketSendNotify send_callback;
    JSocketErrorNotify error_callback;

    void *accept_udata;
    void *error_udata;

    /* send */
    JByteArray *send_data;
    unsigned int send_len;
    void *send_udata;
    /* recv */
    unsigned int recv_len;
    JByteArray *recv_data;
    void *recv_udata;
} JSource;
#define j_source_get_socket(src)    (src)->socket
#define j_source_get_accept_udata(src) (src)->accept_udata
#define j_source_get_send_udata(src)    (src)->send_udata
#define j_source_get_recv_udata(src)    (src)->recv_udata
#define j_source_get_error_udata(src)   (src)->error_udata
#define j_source_get_accept_callback(src)  (src)->accept_callback
#define j_source_get_recv_callback(src)     (src)->recv_callback
#define j_source_get_send_callback(src)     (src)->send_callback
#define j_source_get_error_callback(src)    (src)->error_callback
#define j_source_get_send_data(src)      (src)->send_data
#define j_source_get_send_len(src)  (src)->send_len
#define j_source_get_recv_data(src) (src)->recv_data
#define j_source_get_recv_len(src)  (src)->recv_len
#define j_source_get_events(src)    (src)->events
#define j_source_is_accept(src) (src)->accept

static inline JSource *j_source_new(JSocket * socket);
static inline JSocket *j_source_free(JSource * src);
/* 清楚接受到的数据,以便继续接收 */
static inline void j_source_clear_recv(JSource * src);
static inline void j_source_clear_send(JSource * src);

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop);
static inline int j_main_loop_append_source(JMainLoop * loop,
                                            JSource * src);
static inline void j_main_loop_remove_source(JMainLoop * loop,
                                             JSource * src);
static inline void j_main_loop_update_source(JMainLoop * loop,
                                             JSource * src);
/* */
static inline void j_main_loop_clean_recv(JMainLoop * loop, JSource * src);
static inline void j_main_loop_clean_send(JMainLoop * loop, JSource * src);

/* 查找socket是否已经注册 */
static inline JSource *j_main_loop_find_source(JMainLoop * loop,
                                               JSocket * socket);

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
    while (loop->running && (n = j_poll_wait(poll, events, 1024, 100)) >= 0) {  /* 100 miliseconds */
        int i;
        for (i = 0; i < n; i++) {
            unsigned int evnts = events[i].events;
            JSource *src = (JSource *) events[i].data;
            JSocket *socket = j_source_get_socket(src);
            /* user data */
            void *accept_udata = j_source_get_accept_udata(src);
            void *send_udata = j_source_get_send_udata(src);
            void *recv_udata = j_source_get_recv_udata(src);
            void *error_udata = j_source_get_error_udata(src);
            /* callback */
            JSocketAcceptNotify accept_callback =
                j_source_get_accept_callback(src);
            JSocketSendNotify send_callback =
                j_source_get_send_callback(src);
            JSocketRecvNotify read_callback =
                j_source_get_recv_callback(src);
            JSocketErrorNotify error_callback =
                j_source_get_error_callback(src);
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

            if (evnts & J_POLLHUP || evnts & J_POLLERR) {   /* 监听出错 */
                if (error_callback) {
                    error_callback(socket, error_udata);
                }
                j_main_loop_remove_source(loop, src);
                continue;
            }

            if (evnts & J_POLLIN) { /* 接受数据 */
                if (j_source_is_accept(src)) {
                    JSocket *conn = j_socket_accept(socket);
                    accept_callback(socket, conn, accept_udata);
                } else {
                    JSocketRecvResult *res =
                        j_socket_recv_dontwait(socket, recv_len);

                    j_byte_array_append(recv_data,
                                        j_socket_recv_result_get_data(res),
                                        j_socket_recv_result_get_len(res));
                    if (j_socket_recv_result_get_len(res) < recv_len
                        && j_socket_recv_result_is_normal(res)) {
                        src->recv_len -= j_socket_recv_result_get_len(res);
                    } else {
                        unsigned int count =
                            j_byte_array_get_len(recv_data);
                        void *data = NULL;
                        if (count) {
                            data =
                                j_memdup(j_byte_array_get_data(recv_data),
                                         count);
                        }
                        j_main_loop_clean_recv(loop, src);
                        read_callback(socket,
                                      data,
                                      count,
                                      j_socket_recv_result_get_type
                                      (res), recv_udata);
                        j_free(data);
                    }
                    j_socket_recv_result_free(res);

                    if (j_main_loop_find_source(loop, socket) == NULL) {
                        /* 检查socket是否已经被删除 */
                        continue;
                    }
                }
            }
            if (evnts & J_POLLOUT) {    /* 发送数据 */
                int n = j_socket_send_dontwait(socket,
                                               send_data + send_len,
                                               send_count - send_len);
                if (n > 0) {
                    send_len += n;
                    if (send_len < send_count) {
                        src->send_len = send_len;
                        continue;
                    }
                }

                /* 出错或者读取完成 */
                void *data = j_memdup(send_data, send_len);
                j_main_loop_clean_send(loop, src);
                send_callback(socket, data, send_count, send_len,
                              send_udata);
                j_free(data);

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
    JSource *src = j_main_loop_find_source(loop, sock);
    if (src) {
        return;
    }
    src = j_source_new(sock);
    src->accept_udata = user_data;
    src->accept_callback = notify;
    src->events |= J_POLLIN;
    src->accept = 1;
    if (!j_main_loop_append_source(loop, src)) {
        notify(sock, NULL, user_data);
    }
}

void j_socket_send_async(JSocket * sock, JSocketSendNotify notify,
                         const void *data, unsigned int count,
                         void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src = j_main_loop_find_source(loop, sock);
    if (src == NULL) {
        src = j_source_new(sock);
        src->events |= J_POLLOUT;
        if (!j_main_loop_append_source(loop, src)) {
            notify(sock, data, count, 0, user_data);
        }
    } else if (!(src->events & J_POLLOUT)) {
        src->events |= J_POLLOUT;
        j_main_loop_update_source(loop, src);
    }
    src->send_callback = notify;
    src->send_udata = user_data;
    j_byte_array_append(src->send_data, data, count);
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
    JSource *src = j_main_loop_find_source(loop, sock);
    if (src == NULL) {
        src = j_source_new(sock);
        src->events |= J_POLLIN;
        if (!j_main_loop_append_source(loop, src)) {
            notify(sock, NULL, 0, J_SOCKET_RECV_ERR, user_data);
        }
    } else if (!(src->events & J_POLLIN)) {
        src->events |= J_POLLIN;
        j_main_loop_update_source(loop, src);
    }
    src->recv_callback = notify;
    src->recv_udata = user_data;
    src->recv_len = len;
}


/************************ Mainloop ****************************/

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

static inline void j_source_clear_recv(JSource * src)
{
    src->recv_len = 0;
    j_byte_array_clear(src->recv_data);
}

static inline void j_source_clear_send(JSource * src)
{
    src->send_len = 0;
    j_byte_array_clear(src->send_data);
}

/* 查找socket是否已经注册 */
static inline JSource *j_main_loop_find_source(JMainLoop * loop,
                                               JSocket * socket)
{
    JSource *src = j_hash_table_find(loop->sources, socket);
    return src;
}

void j_socket_set_error_notify(JSocket * sock, JSocketErrorNotify notify,
                               void *user_data)
{
    JMainLoop *loop = j_main_loop_default();
    JSource *src = j_main_loop_find_source(loop, sock);
    if (src) {
        src->error_callback = notify;
    } else {
        src = j_source_new(sock);
        src->error_udata = user_data;
        src->error_callback = notify;
        if (!j_main_loop_append_source(loop, src)) {
            notify(sock, user_data);
        }
    }
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

    unsigned int events = j_source_get_events(src);
    if (!j_poll_ctl(poll, J_POLL_CTL_ADD, events, sock, src)) {
        j_source_free(src);
        return 0;
    }
    j_hash_table_insert(loop->sources, sock, src);
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
        JSocketErrorNotify error_callback =
            j_source_get_error_callback(src);
        void *error_udata = j_source_get_error_udata(src);
        if (error_callback) {
            error_callback(sock, error_udata);
        }
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

/* */
static inline void j_main_loop_clean_recv(JMainLoop * loop, JSource * src)
{
    src->events &= ~J_POLLIN;
    j_source_clear_recv(src);
    j_main_loop_update_source(loop, src);
}

static inline void j_main_loop_clean_send(JMainLoop * loop, JSource * src)
{
    src->events &= ~J_POLLOUT;
    j_source_clear_send(src);
    j_main_loop_update_source(loop, src);
}

/*
 * 遍历所有的socket
 */
typedef struct {
    JMainLoopForeach func;
    void *user_data;
} ForeachData;
static int hash_table_foreach(void *key, void *value, void *user_data)
{
    ForeachData *data = (ForeachData *) user_data;
    JMainLoopForeach func = data->func;
    JSocket *socket = (JSocket *) key;
    JSource *src = (JSource *) value;
    func(socket, j_source_get_events(src), data->user_data);
    return 1;
}

void j_main_loop_foreach_socket(JMainLoop * loop, JMainLoopForeach foreach,
                                void *user_data)
{
    ForeachData data = {
        foreach,
        user_data
    };
    j_hash_table_foreach(loop->sources, hash_table_foreach, &data);
}


/*
 * Finds a socket
 */
JSocket *j_main_loop_find_socket(JMainLoop * loop, JMainLoopFind find,
                                 void *user_data)
{
    JList *keys = j_hash_table_get_keys(loop->sources);
    while (keys) {
        JSocket *socket = (JSocket *) j_list_data(keys);
        if (find(socket, user_data)) {
            return socket;
        }
        keys = j_list_next(keys);
    }
    return NULL;
}

/*
 * 移除指定的socket
 * 如果不存在，则没有任何效果
 */
void j_main_loop_remove_socket(JMainLoop * loop, JSocket * sock)
{
    JSource *src = j_main_loop_find_source(loop, sock);
    if (src == NULL) {
        return;
    }
    j_main_loop_remove_source(loop, src);
}

static inline JSource *j_source_new(JSocket * socket)
{
    JSource *src = (JSource *) j_malloc(sizeof(JSource));
    src->socket = socket;
    src->events = J_POLLERR | J_POLLHUP;
    src->accept_udata = NULL;
    src->send_udata = NULL;
    src->recv_udata = NULL;
    src->accept_callback = NULL;
    src->send_callback = NULL;
    src->recv_callback = NULL;
    src->error_callback = NULL;
    src->error_udata = NULL;
    src->send_data = j_byte_array_new();
    src->send_len = 0;
    src->recv_data = j_byte_array_new();
    src->recv_len = 0;
    src->accept = 0;
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
