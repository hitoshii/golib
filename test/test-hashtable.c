#include <jlib/jlib.h>
#include <stdio.h>
#include <stdlib.h>


jboolean node(void *key, void *value, void *data)
{
    printf("%s:%s\n", (char *) key, (char *) value);
    return TRUE;
}

int main(int argc, char *argv[])
{
    JHashTable *tb =
        j_hash_table_new(5, j_direct_hash, j_direct_equal, NULL, NULL);
    const char *key = "hello";
    j_hash_table_insert(tb, (void *) key, "nice");
    j_hash_table_insert(tb, "nice", "ok");
    j_hash_table_foreach(tb, node, NULL);
    printf("-------------------\n");

    j_hash_table_remove(tb, (void *) key);
    j_hash_table_foreach(tb, node, NULL);
    printf("-------------------\n");

    j_hash_table_free(tb);

    tb = j_hash_table_new(5, j_str_hash, j_str_equal, free, free);

    j_hash_table_insert(tb, j_strdup("well"), j_strdup("done"));
    j_hash_table_insert(tb, j_strdup("done"), j_strdup("yes"));
    j_hash_table_insert(tb, j_strdup("well"), j_strdup("no"));
    j_hash_table_foreach(tb, node, NULL);
    printf("-------------------\n");

    j_hash_table_remove_full(tb, "well");
    j_hash_table_foreach(tb, node, NULL);

    j_hash_table_free_full(tb);

    return 0;
}
