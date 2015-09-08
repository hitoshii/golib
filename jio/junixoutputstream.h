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
#ifndef __JIO_FILE_OUTPUT_STREAM_H__
#define __JIO_FILE_OUTPUT_STREAM_H__

#include "joutputstream.h"
#include "jfile.h"

typedef struct _JUnixOutputStream JUnixOutputStream;

JUnixOutputStream *j_unix_output_stream_open_path(const char * path);

#endif
