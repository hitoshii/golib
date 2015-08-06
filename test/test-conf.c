#include <jconf/jconf.h>


static inline void dump_object(JConfObject * obj, jint tab)
{
    JPtrArray *keys = j_conf_object_get_keys(obj);
    jint i, j;
    for (i = 0; i < j_ptr_array_get_len(keys); i++) {
        const jchar *key = (jchar *) j_ptr_array_get_ptr(keys, i);
        JConfNode *node = j_conf_object_get(obj, key);
        JConfNodeType type = j_conf_node_get_type(node);
        for (j = 0; j < tab; j++) {
            j_printf("\t");
        }
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
        case J_CONF_NODE_TYPE_STRING:
            j_printf("\"%s\"", j_conf_string_get(node));
            break;
        case J_CONF_NODE_TYPE_OBJECT:
            j_printf("{\n");
            dump_object((JConfObject *) node, tab + 1);
            for (j = 0; j < tab; j++) {
                j_printf("\t");
            }
            j_printf("}");
            break;
        default:
            break;
        }
        j_printf("\n");
    }
}

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

    dump_object((JConfObject *) j_conf_loader_get_root(loader), 0);

    j_conf_loader_unref(loader);
    return 0;
}
