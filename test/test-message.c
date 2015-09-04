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

static void log_handler(const char * domain, JLogLevelFlag level,
                        const char * message, void * user_data) {
    printf("[%s] %s\n", domain, message);
}

int main(int argc, char *argv[]) {
    j_log_set_handler(J_LOG_DOMAIN,J_LOG_LEVEL_WARNING|J_LOG_LEVEL_DEBUG, log_handler, NULL);
    j_log(J_LOG_DOMAIN, J_LOG_LEVEL_WARNING, "WARNING!!!!");
    j_debug("这是一个DEBUG，%s", "是吗？");
    j_log_set_handler("test",J_LOG_LEVEL_INFO, log_handler, NULL);
    j_log("test", J_LOG_LEVEL_INFO, "Info!!!!");
    return 0;
}
