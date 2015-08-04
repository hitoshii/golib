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
#include "jconfroot.h"
#include <jlib/jlib.h>

struct _JConfRoot {
    JObject parent;
    JHashTable *nodes;
};

static void j_conf_root_free(JConfRoot * root);

JConfRoot *j_conf_root_new(void)
{
    JConfRoot *root = (JConfRoot *) j_malloc(sizeof(JConfRoot));
    J_OBJECT_INIT(root, j_conf_root_free);
    root->nodes =
        j_hash_table_new(10, j_str_hash, j_str_equal,
                         (JKeyDestroyFunc) j_free,
                         (JValueDestroyFunc) j_object_unref);
    return root;
}


static void j_conf_root_free(JConfRoot * root)
{
    j_hash_table_free_full(root->nodes);
}
