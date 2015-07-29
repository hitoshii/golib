#include <jio/jio.h>

int main(int argc, char const *argv[])
{
    JString *s1 = j_string_new();
    JString *s2 = j_string_new();

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
        j_string_append_len(s1, buf, n);
    }
    if (n != 0) {
        return 3;
    }
    j_object_unref((JObject *) input);

    input = j_file_read(f);
    if (input == NULL) {
        return 4;
    }
    jchar *line;
    while ((line = j_input_stream_readline((JInputStream *) input))) {
        j_printf("%s\n", line);
        j_string_append_printf(s2, "%s\n", line);
        j_free(line);
    }

    if (j_strcmp0(s1->data, s2->data)) {
        return 5;
    }

    juint len = 0;
    jchar *map = j_file_map(f, PROT_READ, MAP_PRIVATE, &len);
    if (map == NULL) {
        return 6;
    }
    if (j_strncmp0(s1->data, map, len)) {
        return 7;
    }
    j_file_unmap(map, len);

    j_object_unref((JObject *) input);
    j_object_unref((JObject *) f);
    j_string_free(s1, TRUE);
    j_string_free(s2, TRUE);
    return 0;
}
