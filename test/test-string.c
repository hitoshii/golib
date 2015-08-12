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

int main(int argc, char *argv[]) {
    char *utf8 = j_str_encode("你们好 ABCDEF 阿!", J_ENCODING_UTF8, 0);
    if (j_strcmp0(utf8, "\\u4F60\\u4EEC\\u597D ABCDEF \\u963F!")) {
        return -1;
    }
    j_free(utf8);
    char **strv = j_strdupv(3, "hello", "world", "everyone");
    char **ptr = strv;
    while (*ptr) {
        printf("%s\n", *ptr);
        ptr++;
    }
    j_strfreev(strv);

    strv = j_strv(4, j_strdup("med"),
                  j_strdup("good"),
                  j_strndup("well done", 100), j_strndup("all right", 5));
    ptr = strv;
    while (*ptr) {
        printf("%s\n", *ptr);
        ptr++;
    }
    j_strfreev(strv);

    if (!(j_str_has_prefix("hello world", "hello")
            && !j_str_has_suffix("no you", "me"))) {
        return 1;
    }

    if (!(j_str_isint("123") || !j_str_isint("22d2"))) {
        return 1;
    }
    if (j_str_toint("232321") != 232321 || j_str_toint("-2921") != -2921) {
        return 1;
    }
    char *str = j_strdup("hello everyone");
    str = j_str_forward(str, 5);
    if (j_strcmp0(str, " everyone")) {
        return 1;
    }
    j_free(str);


    str = j_strdup("Thare is a very good apple");
    str = j_str_replace(str, "is", "are");
    if (j_strcmp0(str, "Thare are a very good apple")) {
        printf("replace error!\n");
        return 1;
    }
    str = j_str_replace(str, "abcdefg", "d");
    if (j_strcmp0(str, "Thare are a very good apple")) {
        printf("replace error!\n");
        return 1;
    }
    j_free(str);

    strv = j_strsplit_c("Hello=Yes= Or No=", '=', 5);
    if (j_strcmp0("Hello", strv[0]) || j_strcmp0("Yes", strv[1]) ||
            j_strcmp0(" Or No", strv[2]) || j_strcmp0("", strv[3])) {
        return 1;
    }
    j_strfreev(strv);

    char *basename = j_path_basename("hello/");
    if (j_strcmp0(basename, "hello")) {
        return 1;
    }
    j_free(basename);
    basename = j_path_basename("/home/wiky/*.conf");
    if (j_strcmp0(basename, "*.conf")) {
        return 1;
    }
    j_free(basename);
    basename = j_path_basename("///");
    if (j_strcmp0(basename, "")) {
        return 1;
    }
    j_free(basename);

    JString *string = j_string_new();
    j_string_append(string, "hello");
    if (j_strcmp0(string->data, "hello")) {
        return 1;
    }
    j_string_append_len(string, "world", 4);
    if (j_strcmp0(string->data, "helloworl")) {
        return 1;
    }
    j_string_append_c(string, 'd');
    if (j_strcmp0(string->data, "helloworld")) {
        return 1;
    }
    j_string_erase(string, 5, -1);
    if (j_strcmp0(string->data, "hello")) {
        return 2;
    }
    j_string_erase(string, 4, 6);
    if (j_strcmp0(string->data, "hell")) {
        return 2;
    }
    j_string_erase(string, 0, 2);
    if (j_strcmp0(string->data, "ll")) {
        return 2;
    }
    j_string_free(string, 1);
    return 0;
}
