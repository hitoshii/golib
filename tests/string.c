#include <jlib.h>
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

    if (!
        (j_str_has_prefix("hello world", "hello")
         && !j_str_has_suffix("no you", "me"))) {
        return 1;
    }

    if (!(j_str_isint("123") || !j_str_isint("22d2"))) {
        return 1;
    }
    if (j_str_toint("232321") != 232321 || j_str_toint ("-2921")!=-2921) {
        return 1;
    }
    char *str=j_strdup("hello everyone");
    str=j_str_forward (str,5);
    if(j_strcmp0 (str," everyone")){
        return 1;
    }
    j_free(str);


    str = j_strdup ("This is a very good apple");
    str = j_str_replace(str,"is","are");
    if(j_strcmp0 (str,"Thare are a very good apple")){
        return 1;
    }
    j_free(str);
    return 0;
}
