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

#include "jtimer.h"
#include <time.h>
#include <errno.h>


/*
 * 睡眠微秒
 * 1秒=1000000微秒
 */
void j_usleep(unsigned long microseconds) {
    struct timespec request, remaining;
    request.tv_sec = microseconds / 1000000;
    request.tv_nsec = 1000 * (microseconds % 1000000);
    while (nanosleep(&request, &remaining) == -1 && errno == EINTR) {
        request = remaining;
    }
}
