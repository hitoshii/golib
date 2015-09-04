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
#include <jlib/jlib.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    JByteArray *array = j_byte_array_new();
    j_byte_array_append(array, (const uint8_t *) "hello", 4);
    j_byte_array_append(array, (const uint8_t *) "o", 2);
    if (j_strcmp0((const char *) j_byte_array_get_data(array), "hello")) {
        return 1;
    }
    void * data = j_byte_array_free(array, 0);
    if (j_strcmp0(data, "hello")) {
        return 2;
    }
    j_free(data);

    array = j_byte_array_sized_new(4);
    j_byte_array_append(array, (const uint8_t *) "hello world", 8);
    if (j_strncmp0
            ((const char *) j_byte_array_get_data(array), "hello", 5)) {
        return 3;
    }
    data = j_byte_array_free(array, 0);
    if (j_strncmp0(data, "hello world", 8)) {
        return 4;
    }
    j_free(data);

    JPtrArray *parray = j_ptr_array_new_full(1, j_free);
    j_ptr_array_append_ptr(parray, j_strdup("hello world"));
    j_ptr_array_append(parray, j_strdup("what?"), j_strdup("nice"),
                       j_strdup("我是谁?"), NULL);
    j_ptr_array_remove_index(parray, 1);
    if (j_strcmp0((char *) j_ptr_array_get(parray, 1), "nice")) {
        return 5;
    }
    j_ptr_array_remove_index_fast(parray, 0);
    if (j_strcmp0((char *) j_ptr_array_get(parray, 0), "我是谁?")) {
        return 6;
    }
    unsigned int i;
    for (i = 0; i < j_ptr_array_get_len(parray); i++) {
        printf("%s\n", (char *) parray->data[i]);
    }
    j_ptr_array_free(parray, TRUE);
    return 0;
}
