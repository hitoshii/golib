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
#include <jmod/jmod.h>
#include <jlib/jlib.h>


int main (int argc, char *argv[]) {
    JacModule *mod = jac_loads_module("./test-mod");
    if(mod==NULL) {
        return -1;
    }
    if(j_strcmp0(mod->name, "name")) {
        return -2;
    }
    if(j_list_length(get_jacques_modules())!=1) {
        return -3;
    }
    if(j_list_length(get_client_accept_hooks())!=0) {
        return -4;
    }
    return 0;
}
