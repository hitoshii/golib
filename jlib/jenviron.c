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
#include "jenviron.h"
#include <stdlib.h>


const jchar *j_getenv(const jchar * variable)
{
    if (J_UNLIKELY(variable == NULL)) {
        return NULL;
    }
    return getenv(variable);
}

jboolean j_setenv(const jchar * variable, const jchar * value,
                  jboolean overwrite)
{
    if (J_UNLIKELY(variable == NULL || value == NULL)) {
        return FALSE;
    }
    return setenv(variable, value, overwrite) == 0;
}
