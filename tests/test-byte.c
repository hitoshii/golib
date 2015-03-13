#include <jlib/jlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    JByteArray *array = j_byte_array_new();
    j_byte_array_append(array, "hello", 4);
    j_byte_array_append(array, "o", 2);
    if (j_strcmp0(j_byte_array_get_data(array), "hello")) {
        return 1;
    }
    void *data = j_byte_array_free(array, 0);
    if (j_strcmp0(data, "hello")) {
        return 1;
    }
    j_free(data);
    return 0;
}
