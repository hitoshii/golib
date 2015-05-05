#include <jconf/jconf.h>
#include <jlib/jlib.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef PACKAGE_TEST_DIR
#define PACKAGE_TEST_DIR "."
#endif

const char *CONFIG_PATH = PACKAGE_TEST_DIR "/test.conf";

static inline void tab_print(unsigned int tab)
{
    int i;
    for (i = 0; i < tab; i++) {
        printf("\t");
    }
}

static inline void jconf_print(JConfNode * node, unsigned int tab)
{
    const char *name = j_conf_node_get_name(node);
    if (name == NULL) {
        name = "";
    }
    tab_print(tab);
    if (j_conf_node_is_string(node)) {
        printf("%s=%s\n", name, j_conf_string_get(node));
    } else if (j_conf_node_is_false(node)) {
        printf("%s=FALSE\n", name);
    } else if (j_conf_node_is_true(node)) {
        printf("%s=TRUE\n", name);
    } else if (j_conf_node_is_int(node)) {
        printf("%s=%ld\n", name, j_conf_int_get(node));
    } else if (j_conf_node_is_float(node)) {
        printf("%s=%g\n", name, j_conf_float_get(node));
    } else if (j_conf_node_is_object(node) || j_conf_node_is_array(node)) {
        JList *children = j_conf_node_get_children(node);
        if (j_conf_node_is_array(node)) {
            printf("%s=[\n", name);
        } else {
            printf("%s={\n", name);
        }
        while (children) {
            JConfNode *child = (JConfNode *) j_list_data(children);
            jconf_print(child, tab + 1);
            children = j_list_next(children);
        }
        tab_print(tab);
        if (j_conf_node_is_array(node)) {
            printf("]\n");
        } else {
            printf("}\n");
        }
    } else {
        printf("%s=NULL\n", name);
    }
}


int main(int argc, char *argv[])
{
    JConfRoot *root = j_conf_root_new(CONFIG_PATH);
    j_conf_root_add_var(root, "CREATOR", "Wiky");
    j_conf_root_add_vars(root, "DATE", "TODAY", "AGE", "123", NULL);
    if (!j_conf_root_load(root)) {
        j_conf_root_free(root);
        printf("%s\n", j_conf_strerror());
        return -1;
    }
    JList *all = j_conf_root_get_children(root);
    while (all) {
        JConfNode *node = (JConfNode *) j_list_data(all);
        jconf_print(node, 0);
        all = j_list_next(all);
    }
    j_conf_root_free(root);
    return 0;
}
