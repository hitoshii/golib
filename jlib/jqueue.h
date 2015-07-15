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
 * License along with the package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
#ifndef __JLIB_QUEUE_H__
#define __JLIB_QUEUE_H__

#include "jlist.h"


typedef struct {
    JList *head;
    JList *tail;
    juint length;
} JQueue;

#define J_QUEUE_INIT    {NULL, NULL, 0}


JQueue *j_queue_new(void);
void j_queue_free(JQueue * queue);
void j_queue_free_full(JQueue * queue, JDestroyNotify destroy);
void j_queue_init(JQueue * queue);
void j_queue_clear(JQueue * queue);
void j_queue_clear_full(JQueue * queue, JDestroyNotify destroy);
jboolean j_queue_is_empty(JQueue * queue);
juint j_queue_get_length(JQueue * queue);
void j_queue_reverse(JQueue * queue);
JQueue *j_queue_copy(JQueue * queue);
void j_queue_foreach(JQueue * queue, JFunc func, jpointer user_data);
JList *j_queue_find(JQueue * queue, jconstpointer data);
JList *j_queue_find_custom(JQueue * queue, JCompareFunc compare,
                           jconstpointer user_data);
void j_queue_sort(JQueue * queue, JCompareDataFunc compare,
                  jpointer user_data);
void j_queue_push_head(JQueue * queue, jpointer data);
void j_queue_push_nth(JQueue * queue, jpointer data, jint n);
void j_queue_push_head_link(JQueue * queue, JList * link);
void j_queue_push_tail(JQueue * queue, jpointer data);
void j_queue_push_tail_link(JQueue * queue, JList * link);
void j_queue_push_nth_link(JQueue * queue, jint n, JList * link);
jpointer j_queue_pop_head(JQueue * queue);
jpointer j_queue_pop_head_link(JQueue * queue);
/*
 * 返回队列的第一个元素
 */
JList *j_queue_peek_head_link(JQueue * queue);
/*
 * 返回队列的最后一个元素
 */
JList *j_queue_peek_tail_link(JQueue * queue);
jpointer j_queue_pop_tail(JQueue * queue);
jpointer j_queue_pop_nth(JQueue * queue, juint n);
JList *j_queue_pop_tail_link(JQueue * queue);
JList *j_queue_pop_nth_link(JQueue * queue, juint n);
JList *j_queue_peek_nth_link(JQueue * queue, juint n);
jint j_queue_link_index(JQueue * queue, JList * link);
void j_queue_unlink(JQueue * queue, JList * link);
void j_queue_delete_link(JQueue * queue, JList * link);
jpointer j_queue_peek_head(JQueue * queue);
jpointer j_queue_peek_tail(JQueue * queue);
jpointer j_queue_peek_nth(JQueue * queue, juint n);
jint j_queue_index(JQueue * queue, jconstpointer data);
jboolean j_queue_remove(JQueue * queue, jconstpointer data);
juint j_queue_remove_all(JQueue * queue, jconstpointer data);
void j_queue_insert_before(JQueue * queue, JList * sibling, jpointer data);
void j_queue_insert_after(JQueue * queue, JList * sibling, jpointer data);
void j_queue_insert_sorted(JQueue * queue, jpointer data,
                           JCompareDataFunc compare, jpointer user_data);

#endif
