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
#include <stdio.h>


static boolean idle(void * data) {
    static int i = 0;
    j_printf("idle - %d\n", i++);
    return TRUE;
}

static boolean timeout(void * data) {
    j_main_loop_quit((JMainLoop *) data);
    return FALSE;
}

int main(int argc, char *argv[]) {
    if (!j_daemonize()) {
        return 1;
    }
    JMainLoop *loop = j_main_loop_new(NULL, FALSE);
    j_timeout_add(1000, timeout, loop);
    j_idle_add(idle, NULL);
    j_main_loop_run(loop);
    j_main_loop_unref(loop);
    return 0;
}
