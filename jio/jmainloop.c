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

#include "jmainloop.h"
#include "jpoll.h"
#include <jlib/jlib.h>


struct _JMainLoop {
    JPoll *poll;

    JHashTable *sources;
};

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop);

/*
 * Allocates memory for JMainLoop structure
 */
static inline JMainLoop *j_main_loop_alloc(JPoll * poll);

/*
 * Creates a new main loop
 */
JMainLoop *j_main_loop_new(void)
{
    JPoll *poll = j_poll_new();
    if (poll == NULL) {
        return NULL;
    }
    return j_main_loop_alloc(poll);
}

/*
 * Runs the main loop
 */
void j_main_loop_run(JMainLoop * loop)
{
    JPoll *poll = j_main_loop_get_poll(loop);
    JPollEvent events[1024];
    int n;
    while ((n = j_poll_wait(poll, events, 1024, 100)) >= 0) {   /* 100 miliseconds */
    }
}


static inline JMainLoop *j_main_loop_alloc(JPoll * poll)
{
    JMainLoop *ml = (JMainLoop *) j_malloc(sizeof(JMainLoop));
    ml->poll = poll;
    ml->sources =
        j_hash_table_new(20, j_direct_hash, j_direct_equal, NULL, NULL);
    return ml;
}

static inline JPoll *j_main_loop_get_poll(JMainLoop * loop)
{
    return loop->poll;
}
