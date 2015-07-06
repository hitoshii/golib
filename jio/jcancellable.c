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
#include "jcancellable.h"
#include <jlib/jlib.h>
#include <jlib/jwakeup.h>

struct _JCancellable {
    juint cancelled:1;
    juint cancelled_running:1;
    juint cancelled_running_waiting:1;

    juint ref;

    juint wakeup_ref;
    JWakeup *wakeup;
};

J_PRIVATE_DEFINE_STATIC(current_cancellable, NULL);
J_MUTEX_DEFINE_STATIC(cancellable_mutex);
J_COND_DEFINE_STATIC(cancellable_cond);

JCancellable *j_cancellable_new(void)
{
    JCancellable *cancellable =
        (JCancellable *) j_malloc(sizeof(JCancellable));
    cancellable->cancelled = FALSE;
    cancellable->cancelled_running = FALSE;
    cancellable->cancelled_running_waiting = FALSE;
    cancellable->ref = 1;
    cancellable->wakeup_ref = 0;
    cancellable->wakeup = NULL;
    return cancellable;
}

static inline void j_cancellable_finalize(JCancellable * cancellable)
{
    if (cancellable->wakeup) {
        j_wakeup_free(cancellable->wakeup);
    }
    j_free(cancellable);
}

void j_cancellable_cancel(JCancellable * cancellable)
{
    j_return_if_fail(cancellable->cancelled == FALSE);

    j_mutex_lock(&cancellable_mutex);
    if (cancellable->cancelled) {
        j_mutex_unlock(&cancellable_mutex);
        return;
    }

    cancellable->cancelled = TRUE;
    cancellable->cancelled_running = TRUE;

    if (cancellable->wakeup) {
        j_wakeup_signal(cancellable->wakeup);
    }

    j_mutex_unlock(&cancellable_mutex);

    j_atomic_int_inc(&cancellable->ref);

    j_mutex_lock(&cancellable_mutex);
    cancellable->cancelled_running = FALSE;
    if (cancellable->cancelled_running_waiting) {
        j_cond_broadcast(&cancellable_cond);
    }
    cancellable->cancelled_running_waiting = FALSE;

    j_mutex_unlock(&cancellable_mutex);
    j_cancellable_unref(cancellable);
}

void j_cancellable_unref(JCancellable * cancellable)
{
    if (j_atomic_int_dec_and_test(&cancellable->ref)) {
        j_cancellable_finalize(cancellable);
    }
}
