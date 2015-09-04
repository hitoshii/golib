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

#include "jmessage.h"
#include "jthread.h"
#include "jhashtable.h"
#include "jmem.h"
#include "jstrfuncs.h"
#include "jprintf.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned int id;
    JLogLevelFlag level;
    JLogFunc func;
    void * data;
    JDestroyNotify destroy; /* 用于释放data */
} JLogHandler;


typedef struct {
    char *name;
    JList *handlers;
} JLogDomain;

static JList *log_domains=NULL;
J_MUTEX_DEFINE_STATIC(log_lock);

static inline void j_log_free(void);

static inline JLogHandler *j_log_handler_new(JLogLevelFlag level, JLogFunc func, void * data, JDestroyNotify destroy) {
    static unsigned int static_id=0;
    if(J_UNLIKELY(static_id==0)) {
        atexit(j_log_free);
    }
    JLogHandler *handler=(JLogHandler*)j_malloc(sizeof(JLogHandler));
    handler->id=++static_id;
    handler->level=level;
    handler->func=func;
    handler->data=data;
    handler->destroy=destroy;
    return handler;
}

static void j_log_handler_free(JLogHandler *handler) {
    if(handler->destroy) {
        handler->destroy(handler->data);
    }
    j_free(handler);
}

static inline JLogDomain *j_log_domain_new(const char *name) {
    JLogDomain *domain=j_malloc(sizeof(JLogDomain));
    domain->name=j_strdup(name);
    domain->handlers=NULL;
    return domain;
}

static inline void j_log_domain_free(JLogDomain *domain) {
    j_free(domain->name);
    j_list_free_full(domain->handlers, (JDestroyNotify)j_log_handler_free);
    j_free(domain);
}

static inline void j_log_free(void) {
    j_mutex_lock(&log_lock);
    j_list_free_full(log_domains, (JDestroyNotify)j_log_domain_free);
    log_domains=NULL;
    j_mutex_unlock(&log_lock);
}

static inline JLogDomain *j_log_get_domain(const char *name) {
    JList *ptr=log_domains;
    while(ptr) {
        JLogDomain *domain=(JLogDomain*)j_list_data(ptr);
        if(j_strcmp0(domain->name, name)==0) {
            return domain;
        }
        ptr=j_list_next(ptr);
    }
    JLogDomain *domain=j_log_domain_new(name);
    log_domains=j_list_append(log_domains, domain);
    return domain;
}

unsigned int j_log_set_handler(const char *domain, JLogLevelFlag level,
                               JLogFunc func, void * user_data) {
    return j_log_set_handler_full(domain, level, func, user_data, NULL);
}

unsigned int j_log_set_handler_full(const char *log_domain, JLogLevelFlag level, JLogFunc func,
                                    void * user_data, JDestroyNotify destroy) {
    j_return_val_if_fail(func!=NULL, 0);
    j_mutex_lock(&log_lock);
    JLogDomain *domain=j_log_get_domain(log_domain);
    JLogHandler *handler=j_log_handler_new(level, func, user_data, destroy);
    domain->handlers=j_list_append(domain->handlers, handler);
    j_mutex_unlock(&log_lock);
    return handler->id;
}


static inline JLogHandler *j_log_find_handler(const char * name, JLogLevelFlag level) {
    JList *ptr=log_domains;
    while(ptr) {
        JLogDomain *domain=(JLogDomain*)j_list_data(ptr);
        if(j_strcmp0(domain->name, name)==0) {
            ptr=domain->handlers;
            JLogHandler *handler=NULL;
            while(ptr) {
                JLogHandler *h=(JLogHandler*)j_list_data(ptr);
                if(h->level & level) {
                    handler=h;
                    break;
                }
                ptr=j_list_next(ptr);
            }
            return handler;
        }
        ptr=j_list_next(ptr);
    }
    return NULL;
}


void j_log_remove_handler(const char * name, unsigned int id) {
    j_mutex_lock(&log_lock);
    JList *ptr=log_domains;
    while(ptr) {
        JLogDomain *d=(JLogDomain*)j_list_data(ptr);
        if(j_strcmp0(name, d->name)==0) {
            ptr=d->handlers;
            while(ptr) {
                JLogHandler *h=(JLogHandler*)j_list_data(ptr);
                if(h->id==id) {
                    d->handlers=j_list_remove(d->handlers, h);
                    j_log_handler_free(h);
                    break;
                }
                ptr=j_list_next(ptr);
            }
            break;
        }
        ptr=j_list_next(ptr);
    }
    j_mutex_unlock(&log_lock);
}

void j_logv(const char * domain, JLogLevelFlag level, const char * fmt,
            va_list ap) {
    j_mutex_lock(&log_lock);
    JLogHandler *handler = j_log_find_handler(domain, level);
    if (handler == NULL || handler->func == NULL) {
        j_mutex_unlock(&log_lock);
        return;
    }
    char *buf=j_strdup_vprintf(fmt, ap);
    handler->func(domain, level, buf, handler->data);
    j_free(buf);
    j_mutex_unlock(&log_lock);
}

void j_log(const char * domain, JLogLevelFlag flag, const char * msg,
           ...) {
    va_list ap;
    va_start(ap, msg);
    j_logv(domain, flag, msg, ap);
    va_end(ap);
}


void j_return_if_fail_warning(const char * domain, const char * at,
                              const char * expression) {
    j_log(domain, J_LOG_LEVEL_WARNING, "%s: assertion '%s' failed",
          at, expression);
}
