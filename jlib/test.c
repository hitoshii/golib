#include "jstring.h"
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
    return 0;
}
