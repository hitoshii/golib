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
#include "jqueue.h"
#include "jmem.h"
#include "jmessage.h"

JQueue *j_queue_new(void)
{
    JQueue *queue = j_malloc0(sizeof(JQueue));
    return queue;
}

void j_queue_free(JQueue * queue)
{
    j_return_if_fail(queue != NULL);

    j_list_free(queue->head);
    j_free(queue);
}

void j_queue_free_full(JQueue * queue, JDestroyNotify destroy)
{
    j_return_if_fail(queue != NULL);

    j_list_free_full(queue->head, destroy);
    j_free(queue);
}

void j_queue_init(JQueue * queue)
{
    j_return_if_fail(queue != NULL);

    queue->head = queue->tail = NULL;
    queue->length = 0;
}

void j_queue_clear(JQueue * queue)
{
    j_return_if_fail(queue != NULL);

    j_list_free(queue->head);
    j_queue_init(queue);
}

jboolean j_queue_is_empty(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, FALSE);
    return queue->length == 0;
}

juint j_queue_get_length(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, 0);
    return queue->length;
}

void j_queue_reverse(JQueue * queue)
{
    j_return_if_fail(queue != NULL);

    queue->tail = queue->head;
    queue->head = j_list_reverse(queue->head);
}

JQueue *j_queue_copy(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    JQueue *result = j_queue_new();
    JList *list = queue->head;
    while (list) {
        j_queue_push_tail(result, list->data);
        list = j_list_next(list);
    }

    return result;
}

void j_queue_foreach(JQueue * queue, JFunc func, jpointer user_data)
{
    j_return_if_fail(queue != NULL);

    JList *list = queue->head;
    while (list) {
        func(j_list_data(list), user_data);
        list = j_list_next(list);
    }
}

JList *j_queue_find(JQueue * queue, jconstpointer data)
{
    j_return_val_if_fail(queue != NULL, NULL);
    return j_list_find(queue->head, data);
}

JList *j_queue_find_custom(JQueue * queue, JCompareFunc compare,
                           jconstpointer user_data)
{
    j_return_val_if_fail(queue != NULL && compare != NULL, NULL);
    return j_list_find_custom(queue->head, compare, user_data);
}

void j_queue_sort(JQueue * queue, JCompareDataFunc compare,
                  jpointer user_data)
{
    j_return_if_fail(queue != NULL && compare != NULL);

    queue->head = j_list_sort_with_data(queue->head, compare, user_data);
    queue->tail = j_list_last(queue->head);
}

void j_queue_push_head(JQueue * queue, jpointer data)
{
    j_return_if_fail(queue != NULL);

    queue->head = j_list_prepend(queue->head, data);
    if (queue->tail == NULL) {
        queue->tail = queue->head;
    }
    queue->length++;
}

void j_queue_push_nth(JQueue * queue, jpointer data, jint n)
{
    j_return_if_fail(queue != NULL);

    if (n < 0 || n >= queue->length) {
        j_queue_push_tail(queue, data);
        return;
    }
    j_queue_insert_before(queue, j_queue_peek_nth_link(queue, n), data);
}

void j_queue_push_head_link(JQueue * queue, JList * link)
{
    j_return_if_fail(queue != NULL && link != NULL &&
                     link->prev == NULL && link->next == NULL);
    link->next = queue->head;
    if (queue->head) {
        queue->head->prev = link;
    } else {
        queue->tail = link;
    }
    queue->head = link;
    queue->length++;
}

void j_queue_push_tail(JQueue * queue, jpointer data)
{
    j_return_if_fail(queue != NULL);

    queue->tail = j_list_append(queue->tail, data);
    if (queue->tail->next) {
        queue->tail = queue->tail->next;
    } else {
        queue->head = queue->tail;
    }
    queue->length++;
}

void j_queue_push_tail_link(JQueue * queue, JList * link)
{
    j_return_if_fail(queue != NULL && link != NULL &&
                     link->prev != NULL && link->next != NULL);

    link->prev = queue->tail;
    if (queue->tail) {
        queue->tail->next = link;
    } else {
        queue->head = link;
    }
    queue->tail = link;
    queue->length++;
}

void j_queue_push_nth_link(JQueue * queue, jint n, JList * link)
{
    j_return_if_fail(queue != NULL && link != NULL);

    if (n < 0 || n >= queue->length) {
        j_queue_push_tail_link(queue, link);
        return;
    }

    JList *next = j_queue_peek_nth_link(queue, n);
    JList *prev = next->prev;

    if (prev) {
        prev->next = link;
    }
    next->prev = link;

    link->next = next;
    link->prev = prev;

    if (queue->head->prev) {
        queue->head = queue->head->prev;
    }
    if (queue->tail->next) {
        queue->tail = queue->tail->next;
    }
    queue->length++;
}

jpointer j_queue_pop_head(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (queue->head) {
        JList *node = queue->head;
        jpointer data = j_list_data(node);

        queue->head = j_list_next(node);
        if (queue->head) {
            queue->head->prev = NULL;
        } else {
            queue->tail = NULL;
        }
        j_list_free1(node, NULL);
        queue->length--;
        return data;
    }
    return NULL;
}

jpointer j_queue_pop_head_link(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (queue->head) {
        JList *node = queue->head;

        queue->head = j_list_next(node);
        if (queue->head) {
            queue->head->prev = NULL;
        } else {
            queue->tail = NULL;
        }
        queue->length--;
        return node;
    }
    return NULL;
}

JList *j_queue_peek_head_link(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    return queue->head;
}

JList *j_queue_peek_tail_link(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    return queue->tail;
}

jpointer j_queue_pop_tail(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (queue->tail) {
        JList *node = queue->tail;
        jpointer data = j_list_data(queue->tail);

        queue->tail = j_list_prev(node);
        if (queue->tail) {
            queue->tail->next = NULL;
        } else {
            queue->head = NULL;
        }
        j_list_free1(queue->tail, NULL);
        queue->length--;
        return data;
    }
    return NULL;
}

jpointer j_queue_pop_nth(JQueue * queue, juint n)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (n >= queue->length) {
        return NULL;
    }

    JList *link = j_queue_peek_nth_link(queue, n);
    jpointer result = j_list_data(link);

    j_queue_delete_link(queue, link);

    return result;
}

JList *j_queue_pop_tail_link(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (queue->tail) {
        JList *node = queue->tail;
        queue->tail = j_list_prev(node);
        if (queue->tail) {
            queue->tail->next = NULL;
            node->prev = NULL;
        } else {
            queue->head = NULL;
        }
        queue->length--;

        return node;
    }
    return NULL;
}

JList *j_queue_pop_nth_link(JQueue * queue, juint n)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (n >= queue->length) {
        return NULL;
    }

    JList *link = j_queue_peek_nth_link(queue, n);
    j_queue_unlink(queue, link);
    return link;
}

JList *j_queue_peek_nth_link(JQueue * queue, juint n)
{
    j_return_val_if_fail(queue != NULL, NULL);

    if (n >= queue->length) {
        return NULL;
    }

    JList *link;
    jint i;
    if (n > queue->length / 2) {
        n = queue->length - n - 1;
        link = queue->tail;
        for (i = 0; i < n; i++) {
            link = j_list_prev(link);
        }
    } else {
        link = queue->head;
        for (i = 0; i < n; i++) {
            link = j_list_next(link);
        }
    }
    return link;
}

jint j_queue_link_index(JQueue * queue, JList * link)
{
    j_return_val_if_fail(queue != NULL, -1);
    return j_list_position(queue->head, link);
}

void j_queue_unlink(JQueue * queue, JList * link)
{
    j_return_if_fail(queue != NULL && link != NULL);
    if (link == queue->tail) {
        queue->tail = queue->tail->prev;
    }
    queue->head = j_list_remove_link(queue->head, link);
    queue->length--;
}

void j_queue_delete_link(JQueue * queue, JList * link)
{
    j_return_if_fail(queue != NULL && link != NULL);

    j_queue_unlink(queue, link);
    j_list_free1(link, NULL);
}

jpointer j_queue_peek_head(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    return queue->head ? queue->head->data : NULL;
}

jpointer j_queue_peek_tail(JQueue * queue)
{
    j_return_val_if_fail(queue != NULL, NULL);

    return queue->tail ? queue->tail->data : NULL;
}

jpointer j_queue_peek_nth(JQueue * queue, juint n)
{
    j_return_val_if_fail(queue != NULL, NULL);

    JList *link = j_queue_peek_nth_link(queue, n);

    if (link) {
        return j_list_data(link);
    }
    return NULL;
}

jint j_queue_index(JQueue * queue, jconstpointer data)
{
    j_return_val_if_fail(queue != NULL, -1);
    return j_list_index(queue->head, data);
}

jboolean j_queue_remove(JQueue * queue, jconstpointer data)
{
    j_return_val_if_fail(queue != NULL, FALSE);

    JList *link = j_list_find(queue->head, data);
    if (link) {
        j_queue_delete_link(queue, link);
    }
    return link != NULL;
}

juint j_queue_remove_all(JQueue * queue, jconstpointer data)
{
    j_return_val_if_fail(queue != NULL, 0);

    juint old_length = queue->length;

    JList *list = queue->head;
    while (list) {
        JList *next = j_list_next(list);
        if (j_list_data(list) == data) {
            j_queue_delete_link(queue, list);
        }
        list = next;
    }
    return old_length - queue->length;
}

void j_queue_insert_before(JQueue * queue, JList * sibling, jpointer data)
{
    j_return_if_fail(queue != NULL);

    if (sibling == NULL) {
        j_queue_push_tail(queue, data);
    } else {
        queue->head = j_list_insert_before(queue->head, sibling, data);
        queue->length++;
    }
}

void j_queue_insert_after(JQueue * queue, JList * sibling, jpointer data)
{
    j_return_if_fail(queue != NULL);
    if (sibling == NULL) {
        j_queue_push_head(queue, data);
    } else {
        j_queue_insert_before(queue, sibling->next, data);
    }
}

void j_queue_insert_sorted(JQueue * queue, jpointer data,
                           JCompareDataFunc compare, jpointer user_data)
{
    j_return_if_fail(queue != NULL);

    JList *list = queue->head;
    while (list && compare(j_list_data(list), data, user_data) < 0) {
        list = j_list_next(list);
    }
    j_queue_insert_before(queue, list, data);
}
