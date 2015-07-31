#include <jio/jio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    JFile *f = j_file_new("/dev/null");

    JOutputStream *output = (JOutputStream *) j_file_write(f);
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
