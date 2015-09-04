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

int main(int argc, char const *argv[]) {
    JString *s1 = j_string_new();
    JString *s2 = j_string_new();

    JFile *f = j_file_new("./" __FILE__);
    if (f == NULL) {
        return 1;
    }
    JFileInputStream *input = j_file_read(f);
    if (input == NULL) {
        return 2;
    }
    char buf[2];
    int n;
    while ((n = j_input_stream_read((JInputStream *) input, buf,
                                    sizeof(buf))) > 0) {
        j_printf("%.*s", n, buf);
        j_string_append_len(s1, buf, n);
    }
    if (n != 0) {
        return 3;
    }
    j_object_unref((JObject *) input);

    input = j_file_read(f);
    JBufferedInputStream *buffered_stream =
        j_buffered_input_stream_new((JInputStream *) input);
    j_file_input_stream_unref(input);
    if (input == NULL) {
        return 4;
    }
    int c;
    while ((c = j_buffered_input_stream_get(buffered_stream)) >0) {
        j_printf("%c", c);
        j_string_append_c(s2, (char)c);
    }

    if (j_strcmp0(s1->data, s2->data)) {
        return 5;
    }

    unsigned int len = 0;
    char *map = j_file_map(f, PROT_READ, MAP_PRIVATE, &len);
    if (map == NULL) {
        return 6;
    }
    if (j_strncmp0(s1->data, map, len)) {
        return 7;
    }
    j_file_unmap(map, len);

    j_buffered_input_stream_unref(buffered_stream);

    j_object_unref((JObject *) f);
    j_string_free(s1, TRUE);
    j_string_free(s2, TRUE);
    return 0;
}
