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
#ifndef __JIO_POLL_H__
#define __JIO_POLL_H__

#include "jioenum.h"
#include <jlib/jlib.h>

int j_poll(struct pollfd *fds, unsigned int nfds, int timeout);

/* poll单个文件描述符 */
int j_poll_simple(int fd, short conditions, int timeout,
                  short * revents);


#endif
