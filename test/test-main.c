#include <jlib/jlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("%ld\n", j_get_monotonic_time());
    printf("%ld\n", j_get_monotonic_time());
    jint64 first = j_get_monotonic_time();
    jint64 last = j_get_monotonic_time();
    if (first > last) {
        printf("%ld,%ld\n", first, last);
        return 1;
    }
    return 0;
}
