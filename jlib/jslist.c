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
#include "jslist.h"
#include "jmem.h"


JSList *j_slist_alloc(jpointer data) {
    JSList *l = (JSList *) j_malloc(sizeof(JSList));
    l->data = data;
    l->next = NULL;
    return l;
}

JSList *j_slist_last(JSList * l) {
    if (l == NULL) {
        return NULL;
    }
    while (j_slist_next(l)) {
        l = j_slist_next(l);
    }
    return l;
}

JSList *j_slist_append(JSList * l, jpointer data) {
    JSList *last = j_slist_last(l);
    if (last == NULL) {
        return j_slist_alloc(data);
    }
    last->next = j_slist_alloc(data);
    return l;
}

JSList *j_slist_preppend(JSList * l, jpointer data) {
    JSList *h = j_slist_alloc(data);
    h->next = l;
    return h;
}

JSList *j_slist_remove(JSList * l, jconstpointer data) {
    if (J_UNLIKELY(l == NULL)) {
        return NULL;
    }
    JSList *tmp;
    if (j_slist_data(l) == data) {
        tmp = j_slist_next(l);
        j_slist_free1(l, NULL);
        return tmp;
    }
    JSList *prev = l;
    tmp = j_slist_next(prev);
    while (tmp) {
        if (j_slist_data(tmp) == data) {
            prev->next = tmp->next;
            j_slist_free1(tmp, NULL);
            break;
        }
        prev = tmp;
        tmp = j_slist_next(prev);
    }
    return l;
}

juint j_slist_length(JSList * l) {
    juint len = 0;
    while (l) {
        len++;
        l = j_slist_next(l);
    }
    return len;
}

JSList *j_slist_find(JSList * l, jpointer data) {
    while (l) {
        if (j_slist_data(l) == data) {
            return l;
        }
        l = j_slist_next(l);
    }
    return NULL;
}

JSList *j_slist_find_custom(JSList * l, JCompareFunc compare,
                            jconstpointer user_data) {
    while (l) {
        if (compare(j_slist_data(l), user_data) == 0) {
            return l;
        }
        l = j_slist_next(l);
    }
    return NULL;
}

jpointer j_slist_find_data_custom(JSList * l, JCompareFunc compare,
                                  jconstpointer user_data) {
    l = j_slist_find_custom(l, compare, user_data);
    if (l) {
        return j_slist_data(l);
    }
    return NULL;
}

void j_slist_free1(JSList * l, JDestroyNotify destroy) {
    if (destroy) {
        destroy(j_slist_data(l));
    }
    j_free(l);
}

void j_slist_free(JSList * l) {
    j_slist_free_full(l, NULL);
}

void j_slist_free_full(JSList * l, JDestroyNotify destroy) {
    while (l) {
        JSList *next = j_slist_next(l);
        j_slist_free1(l, destroy);
        l = next;
    }
}


/*
 * Removes the node link_ from the list and frees it.
 * Compare this to g_slist_remove_link() which removes the node without freeing it.
 */
JSList *j_slist_delete_link(JSList * l, JSList * e) {
    l = j_slist_remove_link(l, e);
    j_slist_free1(e, NULL);
    return l;
}

JSList *j_slist_remove_link(JSList * l, JSList * e) {
    if (l == e) {
        return j_slist_next(l);
    }

    JSList *prev = l;
    JSList *iter = j_slist_next(prev);
    while (iter) {
        if (iter == e) {
            prev->next = j_slist_next(e);
            break;
        }
        prev = iter;
        iter = j_slist_next(prev);
    }
    return l;
}
