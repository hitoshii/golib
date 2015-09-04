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
#ifndef __JLIB_QUEUE_H__
#define __JLIB_QUEUE_H__

#include "jlist.h"


typedef struct {
    JList *head;
    JList *tail;
    unsigned int length;
} JQueue;

#define J_QUEUE_INIT    {NULL, NULL, 0}


JQueue *j_queue_new(void);
void j_queue_free(JQueue * queue);
void j_queue_free_full(JQueue * queue, JDestroyNotify destroy);
void j_queue_init(JQueue * queue);
void j_queue_clear(JQueue * queue);
void j_queue_clear_full(JQueue * queue, JDestroyNotify destroy);
boolean j_queue_is_empty(JQueue * queue);
unsigned int j_queue_get_length(JQueue * queue);
void j_queue_reverse(JQueue * queue);
JQueue *j_queue_copy(JQueue * queue);
void j_queue_foreach(JQueue * queue, JFunc func, void * user_data);
JList *j_queue_find(JQueue * queue, const void * data);
JList *j_queue_find_custom(JQueue * queue, JCompareFunc compare,
                           const void * user_data);
void j_queue_sort(JQueue * queue, JCompareDataFunc compare,
                  void * user_data);
void j_queue_push_head(JQueue * queue, void * data);
void j_queue_push_nth(JQueue * queue, void * data, int n);
void j_queue_push_head_link(JQueue * queue, JList * link);
void j_queue_push_tail(JQueue * queue, void * data);
void j_queue_push_tail_link(JQueue * queue, JList * link);
void j_queue_push_nth_link(JQueue * queue, int n, JList * link);
void * j_queue_pop_head(JQueue * queue);
void * j_queue_pop_head_link(JQueue * queue);
/*
 * 返回队列的第一个元素
 */
JList *j_queue_peek_head_link(JQueue * queue);
/*
 * 返回队列的最后一个元素
 */
JList *j_queue_peek_tail_link(JQueue * queue);
void * j_queue_pop_tail(JQueue * queue);
void * j_queue_pop_nth(JQueue * queue, unsigned int n);
JList *j_queue_pop_tail_link(JQueue * queue);
JList *j_queue_pop_nth_link(JQueue * queue, unsigned int n);
JList *j_queue_peek_nth_link(JQueue * queue, unsigned int n);
int j_queue_link_index(JQueue * queue, JList * link);
void j_queue_unlink(JQueue * queue, JList * link);
void j_queue_delete_link(JQueue * queue, JList * link);
void * j_queue_peek_head(JQueue * queue);
void * j_queue_peek_tail(JQueue * queue);
void * j_queue_peek_nth(JQueue * queue, unsigned int n);
int j_queue_index(JQueue * queue, const void * data);
boolean j_queue_remove(JQueue * queue, const void * data);
unsigned int j_queue_remove_all(JQueue * queue, const void * data);
void j_queue_insert_before(JQueue * queue, JList * sibling, void * data);
void j_queue_insert_after(JQueue * queue, JList * sibling, void * data);
void j_queue_insert_sorted(JQueue * queue, void * data,
                           JCompareDataFunc compare, void * user_data);

#endif
