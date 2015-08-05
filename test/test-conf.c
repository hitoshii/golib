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

    j_conf_loader_unref(loader);
    return 0;
}
