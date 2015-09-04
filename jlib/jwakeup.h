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
#ifndef __JLIB_WAKEUP_H__
#define __JLIB_WAKEUP_H__

#include "jtypes.h"
#include "jxpoll.h"

typedef struct _JWakeup JWakeup;


JWakeup *j_wakeup_new(void);

int j_wakeup_get_pollfd(JWakeup * wakeup, JXPollEvent * e);

/*
 * Acknowledges receipt of a wakeup signal on @wakeup.
 * 收到信号后要读取数据
 */
void j_wakeup_acknowledge(JWakeup * wakeup);

/*
 * Safe to call from a UNIX signal handler
 * As j_wakeup_signal() do not send a true signal
 */
void j_wakeup_signal(JWakeup * wakeup);

void j_wakeup_free(JWakeup * wakeup);

#endif
