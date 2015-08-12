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

#ifndef __J_LIB_STACK_H__
#define __J_LIB_STACK_H__

#include "jtypes.h"


/*
 * A stack implemented by JList
 */

typedef struct _JStack JStack;

JStack *j_stack_new();

/*
 * Pushes a new data into the stack
 */
void j_stack_push(JStack * stack, void *data);

/*
 * Pops the top data from stack
 * If stack is empty, NULL is returned
 */
void *j_stack_pop(JStack * stack);

/*
 * Gets the data that is on the top of stack
 * This function doesn't pop any data
 */
void *j_stack_top(JStack * stack);


/**
 * j_stack_clear:
 * @stack: JStack
 * @destroy: the function to destroy element, or NULL
 *
 * free all elements in stack, and set length to zero
 */
void j_stack_clear(JStack * stack, JDestroyNotify destroy);


/*
 * Gets the length of stack
 */
unsigned int j_stack_length(JStack * stack);

#define j_stack_is_empty(stack) (j_stack_length(stack)==0)


void j_stack_free(JStack * stack, JDestroyNotify destroy);


#endif
