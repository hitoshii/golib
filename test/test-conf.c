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

int main(int argc, char const *argv[]) {
    JConfLoader *loader = j_conf_loader_new();
    j_conf_loader_put_float(loader, "version", 1.3);
    j_conf_loader_put_string(loader, "program", "jacques");
    j_conf_loader_put_integer(loader,"INTEGER1",1);
    j_conf_loader_put_integer(loader, "INTEGER2", 2);
    j_conf_loader_put_integer(loader, "INTEGER20", 20);
    j_conf_loader_allow_unknown_variable(loader, FALSE);

    if(j_conf_loader_loads(loader, "./test.conf")) {
        j_conf_loader_unref(loader);
        return 100;
    }

    jchar *msg=j_conf_loader_build_error_message(loader);
    j_printf("%s\n", msg);
    j_free(msg);

    j_conf_loader_allow_unknown_variable(loader, TRUE);

    if (!j_conf_loader_loads(loader, "./test.conf")) {
        jchar *msg=j_conf_loader_build_error_message(loader);
        j_printf("%s\n",msg);
        j_free(msg);
        j_conf_loader_unref(loader);
        return 1;
    }

    JConfNode *node =
        j_conf_root_get(j_conf_loader_get_root(loader), "wiky");
    if (node == NULL || j_conf_bool_get(node) != TRUE) {
        jchar *msg=j_conf_loader_build_error_message(loader);
        j_printf("%s\n",msg);
        j_free(msg);
        return 2;
    }

    jchar *string=j_conf_node_dump((JConfNode*)j_conf_loader_get_root(loader));
    j_printf("%s", string);
    j_free(string);


    j_conf_loader_unref(loader);
    return 0;
}
