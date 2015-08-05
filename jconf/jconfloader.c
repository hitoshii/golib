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
#include "jconfloader.h"
#include <stdlib.h>

struct _JConfLoader {
    JObject parent;
    JHashTable *env;
    JConfRoot *root;
};

static void j_conf_loader_free(JConfLoader * loader);

JConfLoader *j_conf_loader_new(void)
{
    JConfLoader *loader = (JConfLoader *) j_malloc(sizeof(JConfLoader));
    J_OBJECT_INIT(loader, j_conf_loader_free);
    loader->root = j_conf_root_new();
    loader->env = j_hash_table_new(5, j_str_hash, j_str_equal,
                                   (JKeyDestroyFunc) j_free,
                                   (JValueDestroyFunc) j_object_unref);
    return loader;
}

static void j_conf_loader_free(JConfLoader * loader)
{
    j_hash_table_free_full(loader->env);
    j_conf_root_unref(loader->root);
}

JConfRoot *j_conf_loader_get_root(JConfLoader * loader)
{
    return loader->root;
}

static inline void j_conf_loader_put(JConfLoader * loader,
                                     const jchar * name, JConfNode * node)
{
    j_hash_table_insert(loader->env, j_strdup(name), node);
}

void j_conf_loader_put_integer(JConfLoader * loader, const jchar * name,
                               jint64 integer)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, integer));
}

void j_conf_loader_put_string(JConfLoader * loader, const jchar * name,
                              const jchar * string)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}

void j_conf_loader_put_float(JConfLoader * loader, const jchar * name,
                             jdouble floating)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

void j_conf_loader_put_bool(JConfLoader * loader, const jchar * name,
                            jboolean b)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

void j_conf_loader_put_null(JConfLoader * loader, const jchar * name)
{
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_NULL));
}

typedef enum {
    J_CONF_LOADER_KEY,          /* 下一个应该是一个键值 */
    J_CONF_LOADER_KEY_COLON,    /* 下一个应该是冒号 */
    J_CONF_LOADER_VALUE,
    J_CONF_LOADER_END,
} JConfLoaderState;

#define j_conf_is_space(c) ((c)<=32)
#define j_conf_is_end(c) (j_conf_is_space(c)||(c)==';')


static jchar *j_conf_errors[] = {
    "unknown error",
    "invalid key",
    "invalid value",
    ": required",
    "; required"
};

/*
 * 提取出键值
 * 成功返回键值的长度，并从key返回新分配的键值字符串
 * 失败返回-1，key不会赋值
 *
 * 键值的命名规范同C语言的变量名命名规则，
 * 可以用双引号包含，也可以不用，
 *
 * 比如下面形式是合法的
 *
 * abc
 * a_123
 * "ye_s"
 *
 * 而下面形式是不合法的
 *
 * 1abc
 * "dcd
 */
static inline jint j_conf_fetch_key(const jchar * buf, jchar ** key)
{
    if (!(j_ascii_isalpha(buf[0]) || buf[0] == '_')) {
        return -1;
    }
    jint i = 1;
    while (buf[i] != '\0') {
        if (!(buf[i] == '_' || j_ascii_isalnum(buf[i]))) {
            break;
        }
        i++;
    }
    *key = j_strndup(buf, i);
    return i;
}

static inline jint j_conf_fetch_digit(const jchar * buf,
                                      JConfNode ** value)
{
    jchar *endptr1 = NULL, *endptr2 = NULL;
    jdouble d = strtod(buf, &endptr1);
    jint64 l = strtoll(buf, &endptr2, 10);
    if (d == l) {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, l);
        return endptr2 - buf;
    }
    *value = j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, d);
    return endptr1 - buf;
}

static inline jint j_conf_fetch_value(const jchar * buf,
                                      JConfNode ** value)
{
    if (j_strncmp0(buf, "null", 4) == 0 && j_conf_is_end(buf[4])) {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_NULL);
        return 4;
    } else if (j_strncmp0(buf, "true", 4) == 0 && j_conf_is_end(buf[4])) {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_BOOL, (jboolean) TRUE);
        return 4;
    } else if (j_strncmp0(buf, "false", 5) == 0 && j_conf_is_end(buf[5])) {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_BOOL, (jboolean) FALSE);
        return 5;
    }
    if (j_ascii_isdigit(buf[0])) {
        return j_conf_fetch_digit(buf, value);
    }
    return -2;
}

jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path)
{
    JFileInputStream *input_stream = j_file_input_stream_open(path);
    if (input_stream == NULL) {
        return FALSE;
    }
    JConfLoaderState state = J_CONF_LOADER_KEY;
    jchar *key = NULL, *buf = NULL;
    JConfNode *value = NULL;
    juint line = 0;             /* 记录当前行号 */
    jint i, next;
    JConfRoot *root = j_conf_loader_get_root(loader);
    while ((buf =
            j_input_stream_readline((JInputStream *) input_stream)) !=
           NULL) {
        line++;
        next = 0;
        i = 0;
        while (buf[i] != '\0') {
            if (j_conf_is_space(buf[i])) {
                i++;
                continue;
            }
            switch (state) {
            case J_CONF_LOADER_KEY:
                if ((next = j_conf_fetch_key(buf + i, &key)) <= 0) {
                    goto OUT;
                }
                i += next - 1;
                state = J_CONF_LOADER_KEY_COLON;
                break;
            case J_CONF_LOADER_KEY_COLON:
                if (buf[i] != ':') {
                    next = -3;
                    goto OUT;
                }
                state = J_CONF_LOADER_VALUE;
                break;
            case J_CONF_LOADER_VALUE:
                if ((next = j_conf_fetch_value(buf + i, &value)) <= 0) {
                    goto OUT;
                }
                i += next - 1;
                j_conf_root_set_take(root, key, value);
                key = NULL;
                state = J_CONF_LOADER_END;
                break;
            case J_CONF_LOADER_END:
                if (buf[i] != ';') {
                    next = -4;
                    goto OUT;
                }
                state = J_CONF_LOADER_KEY;
                break;
            }
            i++;
        }
        j_free(buf);
        if (state == J_CONF_LOADER_END) {
            state = J_CONF_LOADER_KEY;
        }
    }
  OUT:
    j_file_input_stream_unref(input_stream);
    j_free(key);
    if (next <= 0) {
        j_fprintf(stderr, "%s:%u error %s\n", path, line,
                  j_conf_errors[-next]);
        return FALSE;
    }
    return TRUE;
}
