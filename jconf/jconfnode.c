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
#include "jconfnode.h"
#include <stdarg.h>
#include <regex.h>

/**
 * SECTION: JConfNode
 * @title: Node of JConf
 * @short_description: a node presents one type of data
 * @include: jconfnode.h
 *
 */

/**
 * JConfNodeType:
 * @J_CONF_NODE_TYPE_NULL: NULL
 * @J_CONF_NODE_TYPE_STRING: char*
 * @J_CONF_NODE_TYPE_INTEGER: int64_t
 * @J_CONF_NODE_TYPE_FLOAT: double
 * @J_CONF_NODE_TYPE_BOOL: boolean
 * @J_CONF_NODE_TYPE_OBJECT: JHashTable<char*, JConfNode*>*
 * @J_CONF_NODE_TYPE_ARRAY: JPtrArray<JConfNode*>
 */

struct _JConfNode {
    JObject parent;
    JConfNodeType type;
    union {
        char *data_string;
        int64_t data_integer;
        double data_floating;
        JHashTable *data_object;
        JPtrArray *data_array;
    } data;
};
#define d_string data.data_string
#define d_integer data.data_integer
#define d_floating data.data_floating
#define d_object data.data_object
#define d_array data.data_array

#define J_CONF_NODE_TYPE(n)  ((n)->type)
#define J_CONF_NODE_IS_ARRAY(n)  (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_ARRAY)
#define J_CONF_NODE_IS_OBJECT(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_OBJECT)
#define J_CONF_NODE_IS_INTEGER(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_INTEGER)
#define J_CONF_NODE_IS_STRING(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_STRING)
#define J_CONF_NODE_IS_NULL(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_NULL)
#define J_CONF_NODE_IS_BOOL(n)   (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_BOOL)
#define J_CONF_NODE_IS_FLOAT(n) (J_CONF_NODE_TYPE(n)==J_CONF_NODE_TYPE_FLOAT)
/**
 * j_conf_node_get_type:
 * @node:
 *
 * Returns: the type of JConfNode
 */
JConfNodeType j_conf_node_get_type(JConfNode * node) {
    return node->type;
}

static void j_conf_node_free(JConfNode * node);

/**
 * j_conf_node_new:
 * @type: type of node
 * @...: depends on @type
 *
 * Creates a new JConfNode with type.
 * For string, integer, float, bool, a value must be specified to initialize the node
 * For object, array, a emptry object or array is created
 *
 * Returns: a new JConfNode. free with j_conf_node_unref()
 */
JConfNode *j_conf_node_new(JConfNodeType type, ...) {
    JConfNode *node = (JConfNode *) j_malloc(sizeof(JConfNode));
    J_OBJECT_INIT(node, j_conf_node_free);
    va_list ap;
    va_start(ap, type);

    node->type = type;
    switch (type) {
    case J_CONF_NODE_TYPE_STRING:
        node->d_string = j_strdup(va_arg(ap, char *));
        break;
    case J_CONF_NODE_TYPE_INTEGER:
        node->d_integer = va_arg(ap, int64_t);
        break;
    case J_CONF_NODE_TYPE_BOOL:
        node->d_integer = va_arg(ap, int) != 0;
        break;
    case J_CONF_NODE_TYPE_FLOAT:
        node->d_floating = va_arg(ap, double);
        break;
    case J_CONF_NODE_TYPE_OBJECT:
        node->d_object =
            j_hash_table_new(6, j_str_hash, j_str_equal,
                             (JKeyDestroyFunc) j_free,
                             (JValueDestroyFunc) j_object_unref);
        break;
    case J_CONF_NODE_TYPE_ARRAY:
        node->d_array =
            j_ptr_array_new_full(0, (JDestroyNotify) j_object_unref);
        break;
    default:
        node->type = J_CONF_NODE_TYPE_NULL;
        break;
    }
    return node;
}

/*
 * 销毁一个结点
 */
static void j_conf_node_free(JConfNode * node) {
    switch (node->type) {
    case J_CONF_NODE_TYPE_STRING:
        j_free(node->d_string);
        break;
    case J_CONF_NODE_TYPE_OBJECT:
        j_hash_table_free_full(node->d_object);
        break;
    case J_CONF_NODE_TYPE_ARRAY:
        j_ptr_array_free(node->d_array, TRUE);
        break;
    default:
        break;
    }
}

/**
 * j_conf_node_is_null:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_NULL
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_null(JConfNode * node) {
    return J_CONF_NODE_IS_NULL(node);
}

/**
 * j_conf_node_is_integer:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_INTEGER
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_integer(JConfNode * node) {
    return J_CONF_NODE_IS_INTEGER(node);
}

/**
 * j_conf_node_is_string:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_INTEGER
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_string(JConfNode * node) {
    return J_CONF_NODE_IS_STRING(node);
}

/**
 * j_conf_node_is_float:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_FLOAT
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_float(JConfNode * node) {
    return J_CONF_NODE_IS_FLOAT(node);
}

/**
 * j_conf_node_is_bool:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_BOOL
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_bool(JConfNode * node) {
    return J_CONF_NODE_IS_BOOL(node);
}

/**
 * j_conf_node_is_object:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_OBJECT
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_object(JConfNode * node) {
    return J_CONF_NODE_IS_OBJECT(node);
}

/**
 * j_conf_node_is_array:
 * @node: JConfNode
 *
 * Checks to see if node has type of J_CONF_NODE_TYPE_ARRAY
 *
 * Returns: TRUE otherwise FALSE
 */
boolean j_conf_node_is_array(JConfNode * node) {
    return J_CONF_NODE_IS_ARRAY(node);
}

/**
 * j_conf_integer_get:
 * @node: JConfInteger
 *
 * Returns: integer value
 */
int64_t j_conf_integer_get(JConfInteger * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_INTEGER(node))) {
        return -1;
    }
    return node->d_integer;
}

/**
 * j_conf_integer_set:
 * @node: JConfInteger
 * @integer: the new integer value
 */
void j_conf_integer_set(JConfInteger * node, int64_t integer) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_INTEGER(node))) {
        return;
    }
    node->d_integer = integer;
}

/**
 * j_conf_string_get:
 * @node: JConfString
 *
 * Returns: string value
 */
const char *j_conf_string_get(JConfString * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_STRING(node))) {
        return NULL;
    }
    return node->d_string;
}

/**
 * j_conf_float_get:
 * @node: JConfFloat
 *
 * Returns: float value
 */
double j_conf_float_get(JConfFloat * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_FLOAT(node))) {
        return 0;
    }
    return node->d_floating;
}

/**
 * j_conf_bool_get:
 * @node: JConfBool
 *
 * Returns: boolean value
 */
boolean j_conf_bool_get(JConfBool * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_BOOL(node))) {
        return FALSE;
    }
    return (boolean) node->d_integer;
}

/**
 * j_conf_array_get_length:
 * @node: JConfArray
 *
 * Returns: length of array
 */
unsigned int j_conf_array_get_length(JConfArray * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_ARRAY(node))) {
        return 0;
    }
    return j_ptr_array_get_len(node->d_array);
}

/**
 * j_conf_array_get:
 * @node: JConfArray
 * @index: index of node
 *
 * Returns: the JConfNode at index
 */
JConfNode *j_conf_array_get(JConfArray * node, unsigned int index) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_ARRAY(node))) {
        return NULL;
    }
    return j_ptr_array_get(node->d_array, index);
}

/**
 * j_conf_array_append:
 * @array: JConfArray
 * @node: the node to append
 *
 * if array is not a valid JConfArray, nothing will be changed
 */
void j_conf_array_append(JConfArray * array, JConfNode * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_ARRAY(array))) {
        return;
    }
    j_ptr_array_append_ptr(array->d_array, node);
}

/**
 * j_conf_array_append_integer:
 * @array: JConfArray
 * @integer: the integer value to append
 */
void j_conf_array_append_integer(JConfArray * array, int64_t integer) {
    j_conf_array_append(array,
                        j_conf_node_new(J_CONF_NODE_TYPE_INTEGER,
                                        integer));
}

/**
 * j_conf_array_append_string:
 * @array: JConfArray
 * @string: the string value to append
 */
void j_conf_array_append_string(JConfArray * array, const char * string) {
    j_conf_array_append(array,
                        j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}


/**
 * j_conf_array_append_bool:
 * @array: JConfArray
 * @b: the boolean value to append
 */
void j_conf_array_append_bool(JConfArray * array, boolean b) {
    j_conf_array_append(array, j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

/**
 * j_conf_array_append_float:
 * @array: JConfArray
 * @floating: the float value to append
 */
void j_conf_array_append_float(JConfArray * array, double floating) {
    j_conf_array_append(array,
                        j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

/**
 * j_conf_object_get:
 * @node: JConfObject
 * @name: the key of child node
 *
 * Returns: JConfNode or NULL if not found
 */
JConfNode *j_conf_object_get(JConfObject * node, const char * name) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_OBJECT(node))) {
        return NULL;
    }
    return (JConfNode *) j_hash_table_find(node->d_object, name);
}

/**
 * j_conf_object_set
 * @node: JConfObject
 * @name: the key name
 * @child: new child node
 */
void j_conf_object_set(JConfObject * node, const char * name,
                       JConfNode * child) {
    j_conf_object_set_take(node, j_strdup(name), child);
}

void j_conf_object_set_take(JConfObject * node, char * name,
                            JConfNode * child) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_OBJECT(node))) {
        return;
    }
    j_hash_table_insert(node->d_object, name, child);
}

/**
 * j_conf_object_set_integer
 * @node: JConfObject
 * @name: the key name
 * @integer: integer value
 */
void j_conf_object_set_integer(JConfObject * node, const char * name,
                               int64_t integer) {
    j_conf_object_set(node, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, integer));
}

void j_conf_object_set_string(JConfObject * node, const char * name,
                              const char * string) {
    j_conf_object_set(node, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}

void j_conf_object_set_bool(JConfObject * node, const char * name,
                            boolean b) {
    j_conf_object_set(node, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

void j_conf_object_set_float(JConfObject * node, const char * name,
                             double floating) {
    j_conf_object_set(node, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

void j_conf_object_set_null(JConfObject * node, const char * name) {
    j_conf_object_set(node, name, j_conf_node_new(J_CONF_NODE_TYPE_NULL));
}

void j_conf_object_remove(JConfObject * node, const char * name) {
    j_hash_table_remove_full(node->d_object, (void *) name);
}

int64_t j_conf_object_get_integer(JConfObject *node, const char *name, int64_t def) {
    JConfNode *child=j_conf_object_get(node, name);
    if(child==NULL||j_conf_node_get_type(child)!=J_CONF_NODE_TYPE_INTEGER) {
        return def;
    }
    return j_conf_integer_get(child);
}

const char *j_conf_object_get_string(JConfObject *node, const char *name, const char *def) {
    JConfNode *child=j_conf_object_get(node, name);
    if(child==NULL||j_conf_node_get_type(child)!=J_CONF_NODE_TYPE_STRING) {
        return def;
    }
    return j_conf_string_get(child);
}

JList *j_conf_object_get_string_list(JConfObject *node, const char *name) {
    JList *strings=NULL;
    JConfNode *child=j_conf_object_get(node, name);
    if(child==NULL) {
        return strings;
    }
    JConfNodeType type=j_conf_node_get_type(child);
    if(type==J_CONF_NODE_TYPE_STRING) {
        return j_list_append(NULL, (void *)j_conf_string_get((JConfString*)child));
    } else if(type==J_CONF_NODE_TYPE_ARRAY) {
        int i, length=j_conf_array_get_length((JConfArray*)child);
        for (int i = 0; i < length; ++i) {
            JConfObject *obj = j_conf_array_get((JConfArray*)child, i);
            if(j_conf_node_is_string(obj)) {
                strings=j_list_append(strings, (void *)j_conf_string_get((JConfString*)obj));
            }
        }
    }
    return strings;
}

boolean j_conf_object_get_bool(JConfObject *node, const char *name, boolean def) {
    JConfNode *child=j_conf_object_get(node, name);
    if(child==NULL||!j_conf_node_is_bool(child)) {
        return def;
    }
    return j_conf_bool_get(child);
}

int64_t j_conf_object_get_integer_priority(JConfObject *root, JConfObject *node,
        const char *name, int64_t def) {
    JConfNode *child;
    if(node!=NULL) {
        child=j_conf_object_get(node, name);
        if(child!=NULL && j_conf_node_is_integer(child)) {
            return j_conf_integer_get(child);
        }
    }
    child=j_conf_object_get(root ,name);
    if(child!=NULL && j_conf_node_is_integer(child)) {
        return j_conf_integer_get(child);
    }
    return def;
}

const char *j_conf_object_get_string_priority(JConfObject *root, JConfObject *node,
        const char *name, const char *def) {
    JConfNode *child;
    if(node!=NULL) {
        child=j_conf_object_get(node, name);
        if(child!=NULL&&j_conf_node_is_string(child)) {
            return j_conf_string_get(child);
        }
    }
    child=j_conf_object_get(root, name);
    if(child!=NULL&&j_conf_node_is_string(child)) {
        return j_conf_string_get(child);
    }
    return def;
}

boolean j_conf_object_get_bool_priority(JConfObject *root, JConfObject *node,
                                        const char *name, boolean def) {
    JConfNode *child;
    if(node!=NULL) {
        child=j_conf_object_get(node, name);
        if(child!=NULL&&j_conf_node_is_bool(child)) {
            return j_conf_bool_get(child);
        }
    }
    child=j_conf_object_get(root,name);
    if(child!=NULL&&j_conf_node_is_bool(child)) {
        return j_conf_bool_get(child);
    }
    return def;
}

JList *j_conf_object_get_string_list_priority(JConfObject *root, JConfObject *node,
        const char *name) {
    JList *strings=j_conf_object_get_string_list(root, name);
    strings=j_list_concat(strings, j_conf_object_get_string_list(node, name));
    return strings;
}

JPtrArray *j_conf_object_get_keys(JConfObject * node) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_OBJECT(node))) {
        return NULL;
    }
    return j_hash_table_get_keys(node->d_object);
}

JList *j_conf_object_lookup(JConfObject *node, const char *reg, JConfNodeType type) {
    if (J_UNLIKELY(!J_CONF_NODE_IS_OBJECT(node))) {
        return NULL;
    }
    regex_t regex;
    if(regcomp(&regex, reg, 0)) {
        return NULL;
    }

    JPtrArray *keys=j_hash_table_get_keys(node->d_object);
    int i, len=j_ptr_array_get_len(keys);
    JList *ret=NULL;
    for(i=0; i<len; i++) {
        const char *key=(const char *)j_ptr_array_get(keys, i);
        JConfNode *child=j_hash_table_find(node->d_object, key);
        if(j_conf_node_get_type(child)==type && !regexec(&regex, key, 0, NULL, 0)) {
            ret=j_list_prepend(ret, (void *)key);
        }
    }

    regfree(&regex);
    return ret;
}


/* 添加缩进 */
static inline void j_conf_node_dump_indent(int indent, JString *string) {
    int i;
    for(i=0; i<indent; i++) {
        j_string_append_c(string, ' ');
    }
}

/* 递归输出字符串  */
static void j_conf_node_dump_internal(JConfNode *node,int indent, JString *string) {
    JConfNodeType type=j_conf_node_get_type(node);
    int i,len;
    JPtrArray *keys;
    switch(type) {
    case J_CONF_NODE_TYPE_INTEGER:
        j_string_append_printf(string, "%ld", j_conf_integer_get(node));
        break;
    case J_CONF_NODE_TYPE_BOOL:
        j_string_append_printf(string, "%s", j_conf_bool_get(node)?"true":"false");
        break;
    case J_CONF_NODE_TYPE_FLOAT:
        j_string_append_printf(string, "%g", j_conf_float_get(node));
        break;
    case J_CONF_NODE_TYPE_STRING:
        j_string_append_printf(string,"\"%s\"", j_conf_string_get(node));
        break;
    case J_CONF_NODE_TYPE_NULL:
        j_string_append(string, "null");
        break;
    case J_CONF_NODE_TYPE_OBJECT:
        j_string_append(string, "{\n");
        keys=j_conf_object_get_keys(node);
        len = j_ptr_array_get_len(keys);
        for(i=0; i<len; i++) {
            const char *key=(const char*)j_ptr_array_get(keys, i);
            JConfNode *child=j_conf_object_get(node, key);
            j_conf_node_dump_indent(indent+4, string);
            j_string_append_printf(string, "%s: ", key);
            j_conf_node_dump_internal(child, indent+4, string);
            j_string_append_c(string, '\n');
        }
        j_conf_node_dump_indent(indent, string);
        j_string_append_c(string, '}');
        break;
    case J_CONF_NODE_TYPE_ARRAY:
        j_string_append(string, "[\n");
        len = j_conf_array_get_length(node);
        for(i=0; i<len; i++) {
            JConfNode *child=j_conf_array_get(node, i);
            j_conf_node_dump_indent(indent+4, string);
            j_conf_node_dump_internal(child, indent+4, string);
            if(i<len-1) {
                j_string_append(string,",\n");
            } else {
                j_string_append_c(string, '\n');
            }
        }
        j_conf_node_dump_indent(indent, string);
        j_string_append_c(string, ']');
        break;
    }
}

/* 转化为字符串格式 */
char *j_conf_node_dump(JConfNode *node) {
    JString *string=j_string_new();
    j_conf_node_dump_internal(node,0, string);
    return j_string_free(string, FALSE);
}
