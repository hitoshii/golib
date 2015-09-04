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
#include "jhashtable.h"
#include "jstrfuncs.h"
#include "jmem.h"
#include "jarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void * key;
    void * value;
} JHashTableNode;

static JHashTableNode *j_hash_table_node_new(void * key, void * value) {
    JHashTableNode *node =
        (JHashTableNode *) j_malloc(sizeof(JHashTableNode));
    node->key = key;
    node->value = value;
    return node;
}

static void j_hash_table_node_free(JHashTableNode * node,
                                   JKeyDestroyFunc key_func,
                                   JValueDestroyFunc value_func) {
    if (key_func) {
        key_func(node->key);
    }
    if (value_func) {
        value_func(node->value);
    }
    j_free(node);
}

/*
 * JHashTable
 */
struct _JHashTable {
    uint32_t size;               /* size of bucket */
    uint32_t mod;                /* a prime number <= size */

    JHashFunc hash_func;
    JEqualFunc equal_func;
    JKeyDestroyFunc key_func;
    JValueDestroyFunc value_func;

    JPtrArray *keys;

    JList **buckets;            /* buckets */
};

/* Each table size has an associated prime modulo (the first prime
 * lower than the table size) used to find the initial bucket. Probing
 * then works modulo 2^n. The prime modulo is necessary to get a
 * good distribution with poor hash functions.
 */
static const uint32_t prime_mod[] = {
    1,                          /* For 1 << 0 */
    2,
    3,
    7,
    13,
    31,
    61,
    127,
    251,
    509,
    1021,
    2039,
    4093,
    8191,
    16381,
    32749,
    65521,                      /* For 1 << 16 */
    131071,
    262139,
    524287,
    1048573,
    2097143,
    4194301,
    8388593,
    16777213,
    33554393,
    67108859,
    134217689,
    268435399,
    536870909,
    1073741789,
    2147483647                  /* For 1 << 31 */
};

/*
 * 31 buckets by default
 */
#define W_HASH_TABLE_DEFAULT_INDEX  (5)

JHashTable *j_hash_table_new(unsigned short index,
                             JHashFunc hash_func,
                             JEqualFunc equal_func,
                             JKeyDestroyFunc key_func,
                             JValueDestroyFunc value_func) {
    /* I found that malloc(sizeof(char*)*(1<<31)) will fail in my system
       but malloc(sizeof(char*)*(1<<30)) not */
    if (index >= 31) {
        index = 31;
    } else if (index < 0) {
        index = W_HASH_TABLE_DEFAULT_INDEX;
    }

    JHashTable *h = (JHashTable *) j_malloc(sizeof(JHashTable));
    h->size = prime_mod[index];
    h->mod = prime_mod[index];
    h->buckets = (JList **) calloc(sizeof(JList *), h->size);   /* init to NULL */
    h->keys = j_ptr_array_new();
    h->equal_func = equal_func;
    h->hash_func = hash_func;
    h->key_func = key_func;
    h->value_func = value_func;

    return h;
}

/*
 * XXX This function iterates over the whole table to count its elements.
 */
unsigned int j_hash_table_length(JHashTable * h) {
    return j_ptr_array_get_len(h->keys);
}

/*
 * return the bucket index of the key
 */
static inline uint32_t j_hash_table_index(JHashTable * h, void * key) {
    uint32_t hash_code = h->hash_func(key);
    uint32_t index = hash_code % h->mod;
    return index;
}

/*
 * return the node that contains given key
 */
static inline JHashTableNode *j_hash_table_find_node(JHashTable * h,
        const void *
        user_data) {
    unsigned int i, len = j_ptr_array_get_len(h->keys);
    void * key = NULL;
    for (i = 0; i < len; ++i) {
        void * data = j_ptr_array_get_ptr(h->keys, i);
        if (h->equal_func(data, user_data) == 0) {
            key = data;
            break;
        }
    }
    if (key == NULL) {
        return NULL;
    }

    uint32_t index = j_hash_table_index(h, key);
    JList *bucket = h->buckets[index];
    JHashTableNode *node = NULL;
    while (bucket) {
        JHashTableNode *data = (JHashTableNode *) bucket->data;
        if (data->key == key) {
            node = data;
            break;
        }
        bucket = j_list_next(bucket);
    }
    return node;
}

boolean j_hash_table_insert(JHashTable * h, void * key, void * value) {
    JHashTableNode *node = j_hash_table_find_node(h, key);
    if (node == NULL) {         /* if not exists, insert */
        uint32_t index = j_hash_table_index(h, key);
        node = j_hash_table_node_new(key, value);
        h->buckets[index] = j_list_append(h->buckets[index], node);
        j_ptr_array_append_ptr(h->keys, node->key);
        return TRUE;
    }
    /* if already exists, update */
    if (h->value_func) {        /* free old value */
        h->value_func(node->value);
    }
    if (h->key_func) {
        h->key_func(key);
    }
    node->value = value;
    return FALSE;
}

boolean j_hash_table_update(JHashTable * h, void * key, void * value) {
    JHashTableNode *node = j_hash_table_find_node(h, key);
    if (node == NULL) {         /* not found */
        return FALSE;
    }
    node->value = value;
    return TRUE;
}

static inline void * j_hash_table_remove_internal(JHashTable * h,
        void * key,
        JKeyDestroyFunc
        key_func,
        JValueDestroyFunc
        value_func) {
    JHashTableNode *node = j_hash_table_find_node(h, key);
    void * value = NULL;

    if (node) {
        uint32_t index = j_hash_table_index(h, key);
        j_ptr_array_remove(h->keys, node->key);
        h->buckets[index] = j_list_remove(h->buckets[index], node);
        if (value_func == NULL) {
            value = node->value;
        }
        j_hash_table_node_free(node, key_func, value_func);
    }
    return value;
}

/* remove but not free key or value */
void * j_hash_table_remove(JHashTable * h, void * key) {
    return j_hash_table_remove_internal(h, key, NULL, NULL);
}

/* remove and free key and value */
void j_hash_table_remove_full(JHashTable * h, void * key) {
    j_hash_table_remove_internal(h, key, h->key_func, h->value_func);
}

void * j_hash_table_find(JHashTable * h, const void * key) {
    JHashTableNode *node = j_hash_table_find_node(h, key);

    if (node) {
        return node->value;
    }

    return NULL;
}

boolean j_hash_table_contains(JHashTable * h, const void * key) {
    return j_hash_table_find_node(h, key) != NULL;
}

void j_hash_table_foreach(JHashTable * h, JNodeFunc node_func,
                          void * data) {
    unsigned int i, len = j_ptr_array_get_len(h->keys);
    for (i = 0; i < len; i++) {
        void * data = j_ptr_array_get_ptr(h->keys, i);
        JHashTableNode *node = j_hash_table_find_node(h, data);
        if (!node_func(node->key, node->value, data)) {
            break;
        }
    }
}

JPtrArray *j_hash_table_get_keys(JHashTable * h) {
    return h->keys;
}


static void j_hash_table_free_internal(JHashTable * h,
                                       JKeyDestroyFunc key_func,
                                       JValueDestroyFunc value_func) {
    uint32_t i;
    for (i = 0; i < h->size; i++) {
        JList *list = h->buckets[i];
        if (list) {
            do {
                JList *next = j_list_next(list);
                j_hash_table_node_free(list->data, key_func, value_func);
                j_list_free1(list, NULL);
                list = next;
            } while (list);
        }
    }
    j_free(h->buckets);
    j_ptr_array_free(h->keys, FALSE);
    j_free(h);
}

void j_hash_table_free_full(JHashTable * h) {
    j_hash_table_free_internal(h, h->key_func, h->value_func);
}

void j_hash_table_free(JHashTable * h) {
    j_hash_table_free_internal(h, NULL, NULL);
}


unsigned int j_str_hash(const void * p) {
    if (p == NULL) {
        return 0;
    }
    const char *s = (const char *) p;
    unsigned int hash = 0;
    while (*s) {
        hash = (hash << 4) + *s;
        s++;
    }
    return hash;
}

int j_str_equal(const void * s1, const void * s2) {
    return j_strcmp0((const char *) s1, (const char *) s2);
}

unsigned int j_int_hash(const void * p) {
    unsigned int i = JPOINTER_TO_JUINT(p);
    return i;
}

int j_int_equal(const void * p1, const void * p2) {
    return (int) (p1 - p2);
}

/* direct */
unsigned int j_direct_hash(const void * p) {
    return JPOINTER_TO_JUINT(p);
}

int j_direct_equal(const void * p1, const void * p2) {
    return (int) (p1 - p2);
}
