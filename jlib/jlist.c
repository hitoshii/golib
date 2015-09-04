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


#include "jlist.h"
#include "jmem.h"
#include "jmessage.h"
#include <stdlib.h>


/*
 * Returns the length of JList
 */
unsigned int j_list_length(JList * l) {
    unsigned int len = 0;
    while (l) {
        len++;
        l = j_list_next(l);
    }
    return len;
}


JList *j_list_append(JList * l, void * data) {
    JList *new = j_list_alloc(data);
    if (l == NULL) {
        return new;
    }
    JList *last = j_list_last(l);
    last->next = new;
    new->prev = last;
    return l;
}

JList *j_list_prepend(JList * l, void * data) {
    JList *new = j_list_alloc(data);
    if (l == NULL) {
        return new;
    }
    JList *first = j_list_first(l);
    first->prev = new;
    new->next = first;
    return new;
}

JList *j_list_first(JList * l) {
    if (l == NULL) {
        return NULL;
    }
    while (j_list_prev(l)) {
        l = j_list_prev(l);
    }
    return l;
}

JList *j_list_last(JList * l) {
    if (l == NULL) {
        return NULL;
    }
    while (j_list_next(l)) {
        l = j_list_next(l);
    }
    return l;
}

JList *j_list_find(JList * l, const void * data) {
    JList *ptr = l;
    while (ptr) {
        if (data == j_list_data(ptr)) {
            return ptr;
        }
        ptr = j_list_next(ptr);
    }
    return NULL;
}

void * j_list_find_custom(JList * l, JCompareFunc compare,
                          const void * user_data) {
    JList *ptr = l;
    while (ptr) {
        const void *data = j_list_data(ptr);
        if (compare(data, user_data) == 0) {
            return ptr;
        }
        ptr = j_list_next(ptr);
    }
    if (ptr) {
        return j_list_data(ptr);
    }
    return NULL;
}


JList *j_list_alloc(void * data) {
    JList *l = (JList *) j_malloc(sizeof(JList));
    l->data = data;
    l->prev = NULL;
    l->next = NULL;
    return l;
}

void j_list_free1(JList * l, JDestroyNotify destroy) {
    if (destroy) {
        destroy(j_list_data(l));
    }
    j_free(l);
}

void j_list_free(JList * l) {
    j_list_free_full(l, NULL);
}

/*
 * Frees the list and all data using JListDestroy
 */
void j_list_free_full(JList * l, JDestroyNotify destroy) {
    if (l == NULL) {
        return;
    }
    do {
        JList *next = j_list_next(l);
        j_list_free1(l, destroy);
        l = next;
    } while (l);
}

int j_list_compare(JList * l1, JList * l2, JCompareFunc compare) {
    while (l1 && l2) {
        int ret = compare(j_list_data(l1), j_list_data(l2));
        if (ret) {
            return ret;
        }
        l1 = j_list_next(l1);
        l2 = j_list_next(l2);
    }
    return l1 == l2;
}


/*
 * Removes an element from a JList.
 * If two or more elements  contain the same data, only the first one is removed.
 * If none of the elements contain the data, JList is unchanged.
 */
JList *j_list_remove(JList * l, void * data) {
    if (l == NULL) {
        return l;
    }
    JList *ptr = l;
    while (ptr) {
        JList *next = j_list_next(ptr);
        if (j_list_data(ptr) == data) {
            JList *prev = j_list_prev(ptr);
            j_list_free1(ptr, NULL);
            if (next) {
                next->prev = prev;
            }
            if (prev == NULL) {
                return next;
            }
            prev->next = next;
            return l;
        }
        ptr = next;
    }
    return l;
}

JList *j_list_remove_link(JList * l, JList * link) {
    if (l == NULL) {
        return l;
    }
    JList *ptr = l;
    while (ptr) {
        JList *next = j_list_next(ptr);
        if (ptr == link) {
            JList *prev = j_list_prev(ptr);
            j_list_free1(ptr, NULL);
            if (next) {
                next->prev = prev;
            }
            if (prev == NULL) {
                return next;
            }
            prev->next = next;
            return l;
        }
        ptr = next;
    }
    return l;
}

/*
 * Removes the node link from the list and frees it
 */
JList *j_list_delete_link(JList * l, JList * link) {
    if (l == NULL || link == NULL) {
        return l;
    }
    JList *ptr = l;
    while (ptr) {
        JList *next = j_list_next(ptr);
        if (ptr == link) {
            JList *prev = j_list_prev(ptr);
            j_list_free1(ptr, NULL);
            if (next) {
                next->prev = prev;
            }
            if (prev == NULL) {
                return next;
            }
            prev->next = next;
            return l;
        }
        ptr = next;
    }
    return l;
}

/*
 * 将双向列表倒转
 */
JList *j_list_reverse(JList * list) {
    JList *last = NULL;
    while (list) {
        last = list;
        list = list->next;
        list->next = last->prev;
        last->prev = list;
    }
    return last;
}

static inline JList *j_list_sort_merge(JList * l1, JList * l2,
                                       JCompareDataFunc compare,
                                       void * user_data) {
    JList list;
    JList *l = &list, *lprev = NULL;

    while (l1 && l2) {
        int cmp = compare(j_list_data(l1), j_list_data(l2), user_data);
        if (cmp <= 0) {
            l->next = l1;
            l1 = j_list_next(l1);
        } else {
            l->next = l2;
            l2 = j_list_next(l2);
        }
        l = j_list_next(l);
        l->prev = lprev;
        lprev = l;
    }
    l->next = l1 ? l1 : l2;
    l->next->prev = l;

    return list.next;
}

static inline JList *j_list_sort_real(JList * list,
                                      JCompareDataFunc compare,
                                      void * user_data) {
    j_return_val_if_fail(list != NULL && list->next != NULL, list);

    JList *l1 = list;
    JList *l2 = j_list_next(list);

    while ((l2 = j_list_next(l2)) != NULL) {
        if ((l2 = j_list_next(l2)) == NULL) {
            break;
        }
        l1 = j_list_next(l1);
    }
    l2 = j_list_next(l1);
    l1->next = NULL;

    return j_list_sort_merge(j_list_sort_real(list, compare, user_data),
                             j_list_sort_real(l2, compare, user_data),
                             compare, user_data);
}

JList *j_list_sort_with_data(JList * list, JCompareDataFunc compare,
                             void * user_data) {
    return j_list_sort_real(list, compare, user_data);
}

/*
 * 查找link在list中的位置
 */
int j_list_position(JList * list, JList * link) {
    int pos = 0;
    while (list) {
        if (list == link) {
            return pos;
        }
        pos++;
        list = j_list_next(list);
    }
    return -1;
}

int j_list_index(JList * list, const void * data) {
    int pos = 0;
    while (list) {
        if (j_list_data(list) == data) {
            return pos;
        }
        pos++;
        list = j_list_next(list);
    }
    return -1;
}

JList *j_list_insert_before(JList * list, JList * sibling, void * data) {
    if (list == NULL) {
        list = j_list_alloc(data);
        j_return_val_if_fail(sibling == NULL, list);
        return list;
    } else if (sibling != NULL) {
        JList *node = j_list_alloc(data);
        node->prev = sibling->prev;
        node->next = sibling;
        sibling->prev = node;

        if (node->prev) {
            node->prev->next = node;
            return list;
        } else {
            j_return_val_if_fail(sibling == list, node);
            return node;
        }
    } else {
        return j_list_append(list, data);
    }
}

JList *j_list_concat(JList *p1, JList *p2) {
    if(p1==NULL) {
        return p2;
    } else if(p2==NULL) {
        return p1;
    }
    JList *last=j_list_last(p1);
    last->next=p2;
    p2->prev=last;
    return p1;
}
