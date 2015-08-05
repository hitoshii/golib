#include <jconf/jconf.h>


int main(int argc, char const *argv[])
{
    JConfLoader *loader = j_conf_loader_new();

    if (!j_conf_loader_loads(loader, "./test.conf")) {
        return 1;
    }

    JConfNode *node =
        j_conf_root_get(j_conf_loader_get_root(loader), "wiky");
    if (node == NULL || j_conf_bool_get(node) != TRUE) {
        return 2;
    }

    JPtrArray *keys = j_conf_root_get_keys(j_conf_loader_get_root(loader));
    jint i;
    for (i = 0; i < j_ptr_array_get_len(keys); i++) {
        const jchar *key = (jchar *) j_ptr_array_get_ptr(keys, i);
        JConfNode *node =
            j_conf_root_get(j_conf_loader_get_root(loader), key);
        JConfNodeType type = j_conf_node_get_type(node);
        j_printf("%s:", key);
        switch (type) {
        case J_CONF_NODE_TYPE_INTEGER:
            j_printf("%ld", j_conf_integer_get(node));
            break;
        case J_CONF_NODE_TYPE_FLOAT:
            j_printf("%f", j_conf_float_get(node));
            break;
        case J_CONF_NODE_TYPE_BOOL:
            j_printf("%s", j_conf_bool_get(node) ? "true" : "false");
            break;
        case J_CONF_NODE_TYPE_NULL:
            j_printf("null");
            break;
        default:
            break;
        }
        j_printf("\n");
    }

    j_conf_loader_unref(loader);
    return 0;
}
