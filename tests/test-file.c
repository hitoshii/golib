#include <jio/jio.h>
#include <jlib/jlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    char *data =
        j_file_readall
        ("/home/wiky/Documents/CODE/Git/jlib/tests/test-file.c");
    printf("%s", data);
    if (data == NULL) {
        return 1;
    }
    j_free(data);

    if (!j_mkdir_with_parents("./hello-dir/world/linux", 0755)) {
        perror("");
        return 1;
    }
    return 0;
}
