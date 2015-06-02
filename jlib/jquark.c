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
#include "jquark.h"
#include "jthread.h"
#include "jhashtable.h"
#include "jmacros.h"
#include "jstrfuncs.h"
#include "jmem.h"
#include "jatomic.h"
#include <string.h>



#define QUARK_BLOCK_SIZE    2048
#define QUARK_STRING_BLOCK_SIZE (4096 - sizeof(jsize))

static inline JQuark quark_new(jchar * string);
static inline jchar *quark_strdup(const jchar * string);
static inline JQuark quark_from_string(const jchar * string,
                                       jboolean duplicate);

J_LOCK_DEFINE_STATIC(quark_global);
static JHashTable *quark_ht = NULL; /* String -> JQuark */
static jchar **quarks = NULL;   /* 保存所有String */
static jint quark_seq_id = 0;
static jchar *quark_block = NULL;
static jint quark_block_offset = 0;


JQuark j_quark_try_string(const jchar * string)
{
    JQuark quark = 0;
    if (string == NULL) {
        return quark;
    }

    J_LOCK(quark_global);
    if (quark_ht) {
        quark = JPOINTER_TO_JUINT(j_hash_table_find(quark_ht, string));
    }
    J_UNLOCK(quark_global);

    return quark;
}

/*
 * 该函数不会复制一个份string，可以节省内存，
 * 但得保证string一直到程序退出都是可访问的
 */
JQuark j_quark_from_static_string(const jchar * string)
{
    JQuark quark = 0;
    if (string == NULL) {
        return quark;
    }

    J_LOCK(quark_global);
    quark = quark_from_string(string, FALSE);
    J_UNLOCK(quark_global);

    return quark;
}

JQuark j_quark_from_string(const jchar * string)
{
    JQuark quark = 0;
    if (string == NULL) {
        return quark;
    }

    J_LOCK(quark_global);
    quark = quark_from_string(string, TRUE);
    J_UNLOCK(quark_global);

    return quark;
}

const jchar *j_quark_to_string(JQuark quark)
{
    jchar *result = NULL;
    jchar **strings = j_atomic_pointer_get(&quarks);
    jint seq_id = j_atomic_int_get(&quark_seq_id);
    if (quark < seq_id) {
        result = strings[quark];
    }
    return result;
}

static inline JQuark quark_new(jchar * string)
{
    JQuark quark;

    if (quark_seq_id % QUARK_BLOCK_SIZE == 0) {
        /* 每次分配QUARK_BLOCK_SIZE大小的内存 */
        jchar **quarks_new = (jchar **) j_realloc(quarks,
                                                  sizeof(jchar *) *
                                                  (quark_seq_id +
                                                   QUARK_BLOCK_SIZE));
        memset(quarks_new + quark_seq_id, 0,
               sizeof(jchar *) * QUARK_BLOCK_SIZE);
        j_atomic_pointer_set(&quarks, quarks_new);
    }

    if (quark_ht == NULL) {     /* quark_seq_id == 0 */
        quark_ht =
            j_hash_table_new(10, j_str_hash, j_str_equal, NULL, NULL);
        quarks[quark_seq_id] = NULL;
        j_atomic_int_inc(&quark_seq_id);
    }

    quark = quark_seq_id;
    j_atomic_pointer_set(&quarks[quark], string);
    j_hash_table_insert(quark_ht, string, JUINT_TO_JPOINTER(quark));
    j_atomic_int_inc(&quark_seq_id);

    return quark;
}

static inline jchar *quark_strdup(const jchar * string)
{
    jchar *copy;
    jsize len;

    len = j_strlen(string) + 1;

    if (len > QUARK_STRING_BLOCK_SIZE / 2) {
        return j_strdup(string);
    }

    if (quark_block == NULL
        || QUARK_STRING_BLOCK_SIZE - quark_block_offset < len) {
        quark_block = j_malloc(QUARK_STRING_BLOCK_SIZE);
        quark_block_offset = 0;
    }

    copy = quark_block + quark_block_offset;
    memcpy(copy, string, len);
    quark_block_offset += len;

    return copy;
}

static inline JQuark quark_from_string(const jchar * string,
                                       jboolean duplicate)
{
    JQuark quark = 0;
    if (quark_ht) {
        quark = JPOINTER_TO_JUINT(j_hash_table_find(quark_ht, string));
    }

    if (quark == 0) {
        quark =
            quark_new(duplicate ? quark_strdup(string) : (jchar *) string);
    }

    return quark;
}
