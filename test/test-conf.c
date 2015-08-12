/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */
#include <jconf/jconf.h>


static inline void dump_array(JConfArray * array, jint tab);
static inline void dump_node(JConfNode * node, jint tab);

static inline void dump_object(JConfObject * obj, jint tab) {
    JPtrArray *keys = j_conf_object_get_keys(obj);
    jint i, j;
    for (i = 0; i < j_ptr_array_get_len(keys); i++) {
        const jchar *key = (jchar *) j_ptr_array_get_ptr(keys, i);
        JConfNode *node = j_conf_object_get(obj, key);
        for (j = 0; j < tab; j++) {
            j_printf("\t");
        }
        j_printf("%s:", key);
        dump_node(node, tab);
    }
}

static inline void dump_node(JConfNode * node, jint tab) {
    jint j;
    JConfNodeType type = j_conf_node_get_type(node);
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
    case J_CONF_NODE_TYPE_ARRAY:
        j_printf("[\n");
        dump_array((JConfArray *) node, tab + 1);
        for (j = 0; j < tab; j++) {
            j_printf("\t");
        }
        j_printf("]");
        break;
    default:
        break;
    }
    j_printf("\n");
}

static inline void dump_array(JConfArray * array, jint tab) {
    jint i, j;
    for (i = 0; i < j_conf_array_get_length(array); i++) {
        JConfNode *node = j_conf_array_get(array, i);
        for (j = 0; j < tab; j++) {
            j_printf("\t");
        }
        dump_node(node, tab);
    }
}

int main(int argc, char const *argv[]) {
    JConfLoader *loader = j_conf_loader_new();
    j_conf_loader_put_float(loader, "version", 1.3);
    j_conf_loader_put_string(loader, "program", "jacques");
    j_conf_loader_put_null(loader, "null");

    if (!j_conf_loader_loads(loader, "./test.conf")) {
        j_printf("%d: %s:%d\n", j_conf_loader_get_errcode(loader),
                 j_conf_loader_get_path(loader),
                 j_conf_loader_get_line(loader));
        j_conf_loader_unref(loader);
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
