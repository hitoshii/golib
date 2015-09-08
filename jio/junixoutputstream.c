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
#include "junixoutputstream.h"
#include <jlib/jlib.h>
#include <fcntl.h>
#include <string.h>


struct _JUnixOutputStream {
    JOutputStream parent;
    int fd;
};

static int j_unix_output_stream_write(JUnixOutputStream * stream,
                                      const char * buf, int len);
static void j_unix_output_stream_close(JUnixOutputStream * stream);

JOutputStreamInterface j_file_output_stream_interface = {
    (JOutputStreamWrite) j_unix_output_stream_write,
    (JOutputStreamClose) j_unix_output_stream_close
};

JUnixOutputStream *j_unix_output_stream_open_path(const char * f) {
    int fd = j_open(f, O_WRONLY);
    if (fd < 0) {
        return NULL;
    }
    JUnixOutputStream *stream = j_malloc(sizeof(JUnixOutputStream));
    j_output_stream_init((JOutputStream *) stream,
                         &j_file_output_stream_interface);
    stream->fd = fd;
    return stream;
}

static int j_unix_output_stream_write(JUnixOutputStream * stream,
                                      const char * buf, int len) {
    if (len < 0) {
        len = strlen(buf);
    }
    return j_write(stream->fd, buf, len);
}

static void j_unix_output_stream_close(JUnixOutputStream * stream) {
    close(stream->fd);
}
