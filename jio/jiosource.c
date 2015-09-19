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
#include "jiosource.h"


struct _JIOSource {
    JSource source;
    int fd;
    short event;
    char *buffer;
    unsigned int size;

    JObject *object;
};

static boolean j_socket_source_dispatch(JSource * source,
                                        JSourceFunc callback,
                                        void * user_data);
static void j_socket_source_finalize(JSource * source);

static JSourceFuncs j_socket_source_funcs = {
    NULL,
    NULL,
    j_io_source_dispatch,
    j_io_source_finalize
};

JIOSource *j_io_source_new(int fd, short event, JObject *object) {
    JIOSource *src =
        (JIOSource *) j_source_new(&j_io_source_funcs,
                                   sizeof(JIOSource));
    src->event = event;
    src->fd=fd;
    src->object=object;
    if(src->object) {
        j_object_ref(src->object);
    }
    j_source_add_poll_fd((JSource *) src,fd, event);
    return src;
}


static boolean j_io_source_dispatch(JSource * source,
                                    JSourceFunc callback,
                                    void * user_data) {
    JIOSource *src = (JIOSource *) source;
    if (src->event & J_XPOLL_OUT) {
        int ret = j_socket_send_with_blocking(src->socket, src->buffer,
                                              src->size, FALSE);
        ((JIOSendCallback) callback) (src->socket, ret, user_data);
    } else if (src->event & J_XPOLL_IN) {
        int ret =
            j_socket_receive_with_blocking(src->socket, src->buffer,
                                           src->size, FALSE);
        return ((JIORecvCallback) callback) (src->socket,
                                             src->buffer, ret,
                                             user_data);
    }
    return FALSE;
}

static void j_io_source_finalize(JSource * source) {
    JIOSource *src = (JIOSource *) source;
    if(src->object) {
        j_object_unref(src->object);
    }
    j_free(src->buffer);
}
