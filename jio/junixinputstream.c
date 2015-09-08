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
#include "junixinputstream.h"
#include <jlib/jlib.h>
#include <fcntl.h>
#include <string.h>

struct _JUnixInputStream {
    JInputStream parent;
    int fd;
};

static int j_unix_input_stream_read(JUnixInputStream * stream,
                                    void *buffer, unsigned int size);
static void j_unix_input_stream_close(JUnixInputStream * stream);

static JInputStreamInterface j_file_input_stream_interface = {
    (JInputStreamRead) j_unix_input_stream_read,
    (JInputStreamClose) j_unix_input_stream_close
};

JUnixInputStream *j_file_input_stream_new_from_fd(int fd) {
    JUnixInputStream *stream = j_malloc(sizeof(JUnixInputStream));
    stream->fd = fd;
    j_input_stream_init((JInputStream *) stream,
                        &j_file_input_stream_interface);
    return stream;
}

JUnixInputStream *j_unix_input_stream_open_path(const char * path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    return j_file_input_stream_new_from_fd(fd);
}

static void j_unix_input_stream_close(JUnixInputStream * stream) {
    close(stream->fd);
}

static int j_unix_input_stream_read(JUnixInputStream * stream,
                                    void *buffer, unsigned int size) {
    return j_read(stream->fd, buffer, size);
}
