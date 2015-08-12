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
#ifndef __J_LIB_MEM_H__
#define __J_LIB_MEM_H__

/*#include "jtypes.h"*/

#include "jtypes.h"


jpointer j_malloc(juint size) J_GNUC_MALLOC;
jpointer j_malloc0(juint size) J_GNUC_MALLOC;
jpointer j_realloc(jpointer mem, juint size);
void j_free(jpointer ptr);

/*
 * 复制一份内存
 */
jpointer j_memdup(jconstpointer data, juint len);


#define j_new(structure, n) j_malloc(sizeof(structure) * n)
#define j_new0(structure, n) j_malloc0(sizeof(structure) *n)



#endif
