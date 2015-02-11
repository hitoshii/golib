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

#ifndef __J_LIB_STACK_H__
#define __J_LIB_STACK_H__


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
 * Gets the length of stack
 */
unsigned int j_stack_length(JStack * stack);

#define j_stack_is_empty(stack) (j_stack_length(stack)==0)


typedef void (*JStackDestroy) (void *data);
void j_stack_free(JStack * stack, JStackDestroy destroy);


#endif
