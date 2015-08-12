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
#include "jfileoutputstream.h"
#include <jlib/jlib.h>
#include <fcntl.h>
#include <string.h>


struct _JFileOutputStream {
    JOutputStream parent;
    int fd;
};

static jint j_file_output_stream_write(JFileOutputStream * stream,
                                       const jchar * buf, jint len);
static void j_file_output_stream_close(JFileOutputStream * stream);

JOutputStreamInterface j_file_output_stream_interface = {
    (JOutputStreamWrite) j_file_output_stream_write,
    (JOutputStreamClose) j_file_output_stream_close
};

JFileOutputStream *j_file_write(JFile * f) {
    jint fd = j_file_open_fd(f, O_WRONLY);
    if (fd < 0) {
        return NULL;
    }
    JFileOutputStream *stream = j_malloc(sizeof(JFileOutputStream));
    j_output_stream_init((JOutputStream *) stream,
                         &j_file_output_stream_interface);
    stream->fd = fd;
    return stream;
}

static jint j_file_output_stream_write(JFileOutputStream * stream,
                                       const jchar * buf, jint len) {
    if (len < 0) {
        len = strlen(buf);
    }
    return j_write(stream->fd, buf, len);
}

static void j_file_output_stream_close(JFileOutputStream * stream) {
    close(stream->fd);
}
