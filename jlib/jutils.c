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
#include "jutils.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

/* 使当前进程成为守护进程 */
jboolean j_daemonize(void)
{
    umask(0);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0 || rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        return FALSE;
    } else if (pid != 0) {      /* 父进程  */
        _exit(0);
    }

    setsid();

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        return FALSE;
    }

    if ((pid = fork()) < 0) {
        return FALSE;
    } else if (pid != 0) {
        _exit(0);
    }

    if (chdir("/") < 0) {
        return FALSE;
    }

    jint i;
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(1);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        return FALSE;
    }
    return TRUE;
}
