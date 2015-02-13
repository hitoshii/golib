#include <jlib/jlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
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


    str = j_strdup("This is a very good apple");
    str = j_str_replace(str, "is", "are");
    if (j_strcmp0(str, "Thare are a very good apple")) {
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
    basename = j_path_basename("/home/wiky/*.conf");
    if (j_strcmp0(basename, "*.conf")) {
        return 1;
    }
    basename = j_path_basename("///");
    if (j_strcmp0(basename, "")) {
        return 1;
    }
    return 0;
}
