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
#include <jio/jio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    JFile *f = j_file_new("/dev/null");

    JOutputStream *output = (JOutputStream *) j_unix_output_stream_open_path(j_file_get_path(f));
    if (output == NULL) {
        return 1;
    }

    if (j_output_stream_write(output, "hello world", -1) != 11) {
        return 2;
    }

    if (j_output_stream_write(output, "yes", 2) != 2) {
        return 3;
    }

    j_output_stream_close(output);

    J_OBJECT_UNREF(output);
    J_OBJECT_UNREF(f);
    return 0;
}
