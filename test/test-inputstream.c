#include <jio/jio.h>

int main(int argc, char const *argv[])
{
    JFile *f = j_file_new("./test-inputstream.c");
    if (f == NULL) {
        return 1;
    }
    JFileInputStream *input = j_file_read(f);
    if (input == NULL) {
        return 2;
    }
    char buf[1024];
    jint n;
    while ((n =
            j_input_stream_read((JInputStream *) input, buf,
                                sizeof(buf))) > 0) {
        j_printf("%.*s", n, buf);
    }
    if (n != 0) {
        return 3;
    }
    j_object_unref((JObject *) input);
    j_object_unref((JObject *) f);
    return 0;
}
