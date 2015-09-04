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
#include <jlib/jlib.h>


int main (int argc, char *argv[]) {
    JModule *mod=j_module_open("./test-mod", J_MODULE_LAZY|RTLD_NODELETE);
    if(mod==NULL) {
        return 1;
    }
    void * ptr;
    int (*sum)(int, int);
    if(!j_module_symbol(mod, "name", &ptr)) {
        return 2;
    }
    if(j_strcmp0(*((const char **)ptr),"nice")) {
        return 3;
    }
    if(!j_module_symbol(mod, "id", &ptr)) {
        return 4;
    }
    if(*((int*)ptr)!=123) {
        return 5;
    }
    if(!j_module_symbol(mod, "scale", &ptr)) {
        return 6;
    }
    if(!j_module_symbol(mod, "sum", (void **)&sum)) {
        return 7;
    }
    j_module_close(mod);
    if(*((float*)ptr)!=0.5) {
        return 8;
    }
    if((*sum)(1,4)!=5) {
        return 9;
    }
    return 0;
}
