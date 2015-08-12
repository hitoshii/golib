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
#include <jlib/jwakeup.h>
#include <stdio.h>
#include <unistd.h>

J_PRIVATE_DEFINE_STATIC(private, NULL);

static jpointer thread_func1(jpointer data) {
    static jchar *once = NULL;
    jboolean init = FALSE;
    if (j_once_init_enter(&once)) {
        j_once_init_leave(&once, "hello all");
        j_printf("once!\n");
        init = TRUE;
    }
    if (!init) {
        return NULL;
    }
    JWakeup *wakeup = (JWakeup *) data;
    sleep(1);
    j_wakeup_signal(wakeup);
    return "hello";
}

static jpointer thread_func(jpointer data) {
    JWakeup *wakeup = j_wakeup_new();
    JThread *thread = j_thread_new("test-thread", thread_func1,
                                   wakeup);
    j_thread_unref(thread);
    thread = j_thread_new("test-thread", thread_func1, wakeup);
    j_thread_unref(thread);
    thread = j_thread_new("test-thread", thread_func1, wakeup);
    j_thread_unref(thread);
    thread = j_thread_new("test-thread", thread_func1, wakeup);
    j_thread_unref(thread);
    thread = j_thread_new("test-thread", thread_func1, wakeup);
    j_thread_unref(thread);

    JEPoll *ep = j_epoll_new();
    JEPollEvent e;
    j_wakeup_get_pollfd(wakeup, &e);
    j_epoll_ctl(ep, e.fd, J_EPOLL_CTL_ADD, e.events, NULL, NULL);
    JEPollEvent event[1];
    jint n = j_epoll_wait(ep, event, 1, -1);
    if (n != 1) {
        return NULL;
    }
    j_wakeup_acknowledge(wakeup);
    j_epoll_close(ep);
    j_wakeup_free(wakeup);
    printf("%s\n", (const jchar *) data);
    return "hello world";
}

int main(int argc, char *argv[]) {
    J_MUTEX_DEFINE(lock);

    j_mutex_lock(&lock);
    j_mutex_unlock(&lock);

    j_mutex_clear(&lock);

    jpointer data = j_private_get(&private);
    if (data != NULL) {
        return 1;
    }
    j_private_set(&private, (jpointer) 1);
    data = j_private_get(&private);
    if (data != (jpointer) 1) {
        return 1;
    }

    JThread *thread = j_thread_new("test-thread", thread_func,
                                   "My name is Jim Green");
    jpointer retval = j_thread_join(thread);
    j_thread_unref(thread);
    if (j_strcmp0(retval, "hello world")) {
        return 1;
    }
    return 0;
}
