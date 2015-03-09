/*
 * Copyright (C) 2015  Wiky L
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with main.c; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef __JIO_MAIN_LOOP_H__
#define __JIO_MAIN_LOOP_H__

#include "jsocket.h"

typedef struct _JMainLoop JMainLoop;


/*
 * Creates a new main loop
 */
JMainLoop *j_main_loop_new(void);

/*
 * Gets the default main loop
 */
JMainLoop *j_main_loop_default(void);

/*
 * Runs the main loop
 */
void j_main_loop_run(JMainLoop * loop);


void j_main_loop_quit(JMainLoop * loop);

void j_main_loop_free(JMainLoop * loop);

/*
 * Runs the default main loop
 */
void j_main(void);
/*
 * Quits the default main loop
 */
void j_main_quit(void);

typedef void (*JSocketAcceptNotify) (JSocket * listen,
                                     JSocket * client, void *user_data);
void j_socket_accept_async(JSocket * sock, JSocketAcceptNotify notify,
                           void *user_data);


#endif
