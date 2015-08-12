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
#include "jutils.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

/* 使当前进程成为守护进程 */
jboolean j_daemonize(void) {
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

static inline jboolean lockfile(jint fd) {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl) == 0;
}

/*
 * 锁定某个文件，如果成功返回文件描述符号，否则返回-1
 */
jint j_lockfile(const jchar * path) {
    jint fd = open(path, O_RDWR | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        return -1;
    }
    if (!lockfile(fd)) {
        close(fd);
        return -1;
    }
    return fd;
}
