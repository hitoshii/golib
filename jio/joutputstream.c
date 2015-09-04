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
#include "joutputstream.h"

void j_output_stream_init(JOutputStream * stream,
                          JOutputStreamInterface * interface) {
    J_OBJECT_INIT(stream, j_output_stream_close);
    stream->iface = interface;
    stream->closed = FALSE;
}

int j_output_stream_write(JOutputStream * stream, const char * buf,
                          int len) {
    if (stream->iface->write) {
        return stream->iface->write(stream, buf, len);
    }
    return 0;
}

void j_output_stream_close(JOutputStream * stream) {
    if (stream->closed == FALSE && stream->iface->close) {
        stream->iface->close(stream);
        stream->closed = TRUE;
    }
}
