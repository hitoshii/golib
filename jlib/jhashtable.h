/*
 * Copyright (C) 2014  Wiky L
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

#ifndef __JLIB_HASHTABLE_H__
#define __JLIB_HASHTABLE_H__

#include "jlist.h"
/*
 * JHashTable, a general hash table.
 * Remember that JHashTable never copys key or value, which means they should 
 * be accessed for the lifetime of JHashTable
 */

/*
 * The structure of WHastTable is not public
 */
typedef struct _JHashTable JHashTable;


/*
 * The hash function
 * 
 * @return: the hash code
 */
typedef juint(*JHashFunc) (jconstpointer data);
/*
 * The equal function
 * 
 * @return: zero if equal, non-zero if not.
 */
typedef JCompareFunc JEqualFunc;
/*
 * the key & value destroy function
 */
typedef JDestroyNotify JKeyDestroyFunc;
typedef JDestroyNotify JValueDestroyFunc;

/*
 *
 */
typedef jint(*JNodeFunc) (jpointer key, jpointer value,
                          jpointer user_data);

/*
 * create a new WHastTable
 */
JHashTable *j_hash_table_new(jushort i,
                             JHashFunc hash_func,
                             JEqualFunc equal_func,
                             JKeyDestroyFunc key_func,
                             JValueDestroyFunc value_func);

/*
 * @description: Insert a new key:value into hash table
 *				 if the key already exists, just update its value
 * @param h: the hash table
 * @param key: the key
 * @param value: the value
 * 
 * @return: TRUE if inserted, FALSE if updated
 */
jboolean j_hash_table_insert(JHashTable * h, jpointer key, jpointer value);

/*
 * @description: Update the value associated to the given key,
 *				if not exists, do nothing
 * 
 * @param h: the hash table
 * @param key: the key
 * @param value: the new value of key.
 * 
 * @return: TRUE if updated, FALSE if key not found
 */
jboolean j_hash_table_update(JHashTable * h, jpointer key, jpointer value);

/*
 * @description: Remove a node whoes key matches.
 *				 this function does not free key and value.
 * 
 * @param key: the key to remove.
 * 
 * @return: the value found or NULL
 */
jpointer j_hash_table_remove(JHashTable * h, jpointer key);

/* remove and free */
void j_hash_table_remove_full(JHashTable * h, jpointer key);


/*
 * @description: find the value associated to to key
 * 
 * @return: the value if found, or NULL
 */
jpointer j_hash_table_find(JHashTable * h, jconstpointer key);


/*
 * Checks to see if the key is in h
 */
jboolean j_hash_table_contains(JHashTable *h, jconstpointer key);

/*
 * @description: call node_func on every element in the hash table.
 */
void j_hash_table_foreach(JHashTable * h, JNodeFunc node_func,
                          jpointer data);

/*
 * @description: get all the keys in the hash table
 * 
 * @return: the list of keys, this list is owned by hash table,
 *          should not be modified.
 */
JList *j_hash_table_get_keys(JHashTable * h);


/*
 * @description: free a hash table
 *				 this function will call WKeyDestroyFunc and 
 *				 WValueDestroyFunc on every element.
 */
void j_hash_table_free_full(JHashTable * h);

/*
 * @descrition: just free hash table itself
 */
void j_hash_table_free(JHashTable * h);


/*
 * hash and equal functions
 */

/* string */
juint j_str_hash(jconstpointer p);
jint j_str_equal(jconstpointer s1, jconstpointer s2);

/* int */
juint j_int_hash(jconstpointer p);
jint j_int_equal(jconstpointer p1, jconstpointer p2);

/* direct */
juint j_direct_hash(jconstpointer p);
jint j_direct_equal(jconstpointer p1, jconstpointer p2);


#endif
