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

#include "jstack.h"
#include "jlist.h"
#include "jmem.h"

struct _JStack {
    unsigned int len;           /* current count of elements in stack */
    JList *data;
};


JStack *j_stack_new()
{
    JStack *stack = (JStack *) j_malloc(sizeof(JStack));
    stack->data = NULL;
    stack->len = 0;
    return stack;
}

/*
 * Pushes a new data into the stack
 */
void j_stack_push(JStack * stack, void *data)
{
    stack->data = j_list_prepend(stack->data, data);
    stack->len++;
}

/*
 * Pops the top data from stack
 * If stack is empty, NULL is returned
 */
void *j_stack_pop(JStack * stack)
{
    if (stack->data == NULL) {
        return NULL;
    }
    JList *next = j_list_next(stack->data);
    if (next) {
        next->prev = NULL;
    }
    void *data = j_list_data(stack->data);
    j_list_free1(stack->data, NULL);

    stack->data = next;
    stack->len--;
    return data;
}

/*
 * Gets the data that is on the top of stack
 * This function doesn't pop any data
 */
void *j_stack_top(JStack * stack)
{
    if (stack->data == NULL) {
        return NULL;
    }
    return j_list_data(stack->data);
}

unsigned int j_stack_length(JStack * stack)
{
    return stack->len;
}

void j_stack_free(JStack * stack, JDestroyNotify destroy)
{
    j_list_free_full(stack->data, (JDestroyNotify) destroy);
    j_free(stack);
}
