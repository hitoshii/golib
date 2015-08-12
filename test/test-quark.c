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


int main(int argc, char *argv[]) {
    JQuark q0 = j_quark_try_string("nice");
    JQuark q1 = j_quark_from_string("nice");
    JQuark q2 = j_quark_from_static_string("nice");
    if (q1 != q2 || q0 != 0) {
        return -1;
    }
    j_printf("%d:%d:%d\n", q0, q1, q2);
    return 0;
}
