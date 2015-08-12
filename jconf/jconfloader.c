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
#include "jconfloader.h"
#include <stdlib.h>


struct _JConfLoader {
    JObject parent;
    JHashTable *env;
    JConfRoot *root;

    /* 解析信息 */
    JStack *info;
    JConfLoaderInfo *top;       /* 栈顶元素，用于快速访问 */
};

static inline JConfLoaderInfo *j_conf_loader_info_new(const jchar *
        filename);
static inline void j_conf_loader_info_free(JConfLoaderInfo * info);

static void j_conf_loader_free(JConfLoader * loader);

static inline jboolean j_conf_loader_loads_object(JConfLoader * loader,
        JConfObject * object,
        JBufferedInputStream *
        input_stream,
        jboolean is_root);
static inline jboolean j_conf_loader_loads_array(JConfLoader * loader,
        JConfArray * array,
        JBufferedInputStream *
        buffered_stream);
static inline jboolean j_conf_loader_loads_from_path(JConfLoader * loader,
        JConfObject * object,
        const jchar * path,
        jboolean is_root);

static inline void j_conf_loader_push_line(JConfLoader * loader,
        JBufferedInputStream *
        buffered_stream, jchar * buf);
static inline void j_conf_loader_set_errcode(JConfLoader * loader,
        jint errcode);
static inline void j_conf_loader_inc_line(JConfLoader * loader);
static inline void j_conf_loader_dec_line(JConfLoader * loader);

static inline jboolean j_conf_loader_include(JConfLoader * loader,
        JConfObject * root,
        JConfNode * node);

JConfLoader *j_conf_loader_new(void) {
    JConfLoader *loader = (JConfLoader *) j_malloc(sizeof(JConfLoader));
    J_OBJECT_INIT(loader, j_conf_loader_free);
    loader->root = j_conf_root_new();
    loader->env = j_hash_table_new(5, j_str_hash, j_str_equal,
                                   (JKeyDestroyFunc) j_free,
                                   (JValueDestroyFunc) j_object_unref);
    loader->info = j_stack_new();
    loader->top = NULL;
    return loader;
}

static void j_conf_loader_free(JConfLoader * loader) {
    j_hash_table_free_full(loader->env);
    j_stack_free(loader->info, (JDestroyNotify) j_conf_loader_info_free);
    j_conf_root_unref(loader->root);
}

JConfRoot *j_conf_loader_get_root(JConfLoader * loader) {
    return loader->root;
}

static inline void j_conf_loader_put(JConfLoader * loader,
                                     const jchar * name, JConfNode * node) {
    j_hash_table_insert(loader->env, j_strdup(name), node);
}

JConfNode *j_conf_loader_get(JConfLoader * loader, const jchar * name) {
    JConfNode *node = j_hash_table_find(loader->env, name);
    return node;
}

void j_conf_loader_put_integer(JConfLoader * loader, const jchar * name,
                               jint64 integer) {
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, integer));
}

void j_conf_loader_put_string(JConfLoader * loader, const jchar * name,
                              const jchar * string) {
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_STRING, string));
}

void j_conf_loader_put_float(JConfLoader * loader, const jchar * name,
                             jdouble floating) {
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_FLOAT, floating));
}

void j_conf_loader_put_bool(JConfLoader * loader, const jchar * name,
                            jboolean b) {
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_BOOL, b));
}

void j_conf_loader_put_null(JConfLoader * loader, const jchar * name) {
    j_conf_loader_put(loader, name,
                      j_conf_node_new(J_CONF_NODE_TYPE_NULL));
}

typedef enum {
    J_CONF_LOADER_KEY = 0,      /* 下一个应该是一个键值 */
    J_CONF_LOADER_KEY_COLON,    /* 下一个应该是冒号 */
    J_CONF_LOADER_VALUE,
    J_CONF_LOADER_END,
    J_CONF_LOADER_ARRAY_VALUE,
    J_CONF_LOADER_ARRAY_DOT,
} JConfLoaderState;

#define j_conf_is_space(c) ((c)<=32)
#define j_conf_is_end(c) (j_conf_is_space(c)||(c)==';')


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
static inline jint j_conf_loader_fetch_key(JConfLoader * loader,
        const jchar * buf, jchar ** key) {
    jboolean quote = FALSE;
    jint i = 0;
    if (buf[0] == '\"') {
        quote = TRUE;
        i++;
    }
    if (!(j_ascii_isalpha(buf[i]) || buf[i] == '_')) {
        goto ERROR;
    }
    i++;
    while (buf[i] != '\0') {
        if (!(buf[i] == '_' || j_ascii_isalnum(buf[i]))) {
            break;
        }
        i++;
    }
    if (quote) {
        if (buf[i] != '\"') {
            goto ERROR;
        }
        *key = j_strndup(buf + 1, i - 1);
        return i + 1;
    }
    *key = j_strndup(buf, i);
    return i;
ERROR:
    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_KEY);
    return -1;
}


/* 获取数字值，该函数不会返回失败 */
static inline jint j_conf_loader_fetch_digit(JConfLoader * loader,
        const jchar * buf,
        JConfNode ** value) {
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

/*
 * 解析四个字节表示的16进制数
 */
static inline juint32 parse_hex4(const jchar * str) {
    juint h = 0;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    return h;
}


/*
 * 获取字符串值
 */
static inline jint j_conf_loader_fetch_string(JConfLoader * loader,
        const jchar * buf,
        JConfNode ** value) {
    static const juchar firstByteMark[7] = {
        0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
    };
    JString *out = NULL;
    if (J_UNLIKELY(buf[0] != '\"')) {
        goto ERROR;
    }
    jchar *ptr = (jchar *) (buf + 1);
    jint len = 0;
    while (*ptr != '\"') {
        if (*ptr == '\0') {
            goto ERROR;
        } else if (*ptr++ == '\\') {
            ptr++;
        }
        len++;
    }
    const jchar *start = buf++;
    out = j_string_new_with_length(128);
    juint32 uc, uc2;
    while (*buf != '\"') {
        if (*buf == '$') {
            jchar *key = NULL;
            jint ret = j_conf_loader_fetch_key(loader, buf + 1, &key);
            if (ret < 0) {
                goto ERROR;
            }
            JConfNode *node = j_conf_loader_get(loader, key);
            j_free(key);
            if (node) {
                JConfNodeType type = j_conf_node_get_type(node);
                if (type == J_CONF_NODE_TYPE_STRING) {
                    j_string_append(out, j_conf_string_get(node));
                } else if (type == J_CONF_NODE_TYPE_FLOAT) {
                    j_string_append_printf(out, "%f",
                                           j_conf_float_get(node));
                } else if (type == J_CONF_NODE_TYPE_INTEGER) {
                    j_string_append_printf(out, "%ld",
                                           j_conf_integer_get(node));
                } else if (type == J_CONF_NODE_TYPE_BOOL) {
                    j_string_append(out,
                                    j_conf_bool_get(node) ? "true" :
                                    "false");
                }
            }
            buf += ret;
        } else if (*buf != '\\') {
            j_string_append_c(out, *buf);
        } else {
            buf++;
            switch (*buf) {
            case 'b':
                j_string_append_c(out, '\b');
                break;
            case 'f':
                j_string_append_c(out, '\f');
                break;
            case 'n':
                j_string_append_c(out, '\n');
                break;
            case 'r':
                j_string_append_c(out, '\r');
                break;
            case 't':
                j_string_append_c(out, '\t');
                break;
            case 'u':
                uc = parse_hex4(buf + 1);
                buf += 4;
                if (uc == 0 || (uc > 0xDC00 && uc <= 0xDFFF)) {
                    goto ERROR;
                } else if (uc >= 0xD800 && uc <= 0xD8FF) {
                    if (buf[1] != '\\' || buf[2] != 'u') {
                        goto ERROR;
                    }
                    uc2 = parse_hex4(buf + 3);
                    buf += 6;
                    if (uc2 < 0xDC00 || uc2 > 0xDFFF) {
                        goto ERROR;
                    }
                    uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                }
                len = 4;
                if (uc < 0x80) {
                    len = 1;
                } else if (uc < 0x800) {
                    len = 2;
                } else if (uc < 0x10000) {
                    len = 3;
                }
                char bytes[4];
                switch (len) {
                case 4:
                    bytes[3] = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 3:
                    bytes[2] = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 2:
                    bytes[1] = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 1:
                    bytes[0] = (uc | firstByteMark[len]);
                }
                j_string_append_len(out, bytes, len);
                break;
            default:
                j_string_append_c(out, *buf);
            }
        }
        buf++;
    }
    ptr = j_string_free(out, FALSE);
    *value = j_conf_node_new(J_CONF_NODE_TYPE_STRING, ptr);
    j_free(ptr);
    return buf + 1 - start;
ERROR:
    j_string_free(out, TRUE);
    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_STRING);
    return -1;
}

/* 获取变量值给value */
static inline jint j_conf_loader_fetch_variable(JConfLoader * loader,
        const jchar * buf,
        JConfNode ** value) {
    if (J_UNLIKELY(buf[0] != '$')) {
        return -1;
    }
    jchar *key = NULL;
    jint ret = j_conf_loader_fetch_key(loader, buf + 1, &key);
    if (ret <= 0) {
        return -1;
    }
    JConfNode *node = j_conf_loader_get(loader, key);
    j_free(key);
    if (node) {
        j_conf_node_ref(node);
        *value = node;
    } else {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_NULL);
    }
    return ret + 1;
}

/*
 * 解析值,如果成功返回，返回被提取的字符串的长度
 * 如果失败，返回-1
 */
static inline jint j_conf_loader_fetch_value(JConfLoader * loader,
        const jchar * buf,
        JConfNode ** value) {
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
        return j_conf_loader_fetch_digit(loader, buf, value);
    } else if (buf[0] == '\"') {
        return j_conf_loader_fetch_string(loader, buf, value);
    } else if (buf[0] == '{') {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_OBJECT);
        return 1;
    } else if (buf[0] == '[') {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_ARRAY);
        return 1;
    } else if (buf[0] == '$') {
        return j_conf_loader_fetch_variable(loader, buf, value);
    }
    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_VALUE);
    return -1;
}

jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path) {
    j_stack_clear(loader->info, (JDestroyNotify) j_conf_loader_info_free);
    return j_conf_loader_loads_from_path(loader, (JConfObject *)
                                         j_conf_loader_get_root(loader),
                                         path, TRUE);
}

/*
 * 读取配置文件，载入到JConfObject中
 */
static inline jboolean j_conf_loader_loads_from_path(JConfLoader * loader,
        JConfObject * object,
        const jchar * path,
        jboolean is_root) {
    loader->top = j_conf_loader_info_new(path);
    j_stack_push(loader->info, loader->top);
    JFileInputStream *input_stream = j_file_input_stream_open(path);
    if (input_stream == NULL) {
        loader->top->errcode=J_CONF_LOADER_ERR_INVALID_FILE;
        return FALSE;
    }
    JBufferedInputStream *buffered_stream =
        j_buffered_input_stream_new((JInputStream *) input_stream);
    jboolean ret = j_conf_loader_loads_object(loader, object,
                   buffered_stream, is_root);
    j_file_input_stream_unref(input_stream);
    j_buffered_input_stream_unref(buffered_stream);
    if (!is_root && ret == TRUE) {
        /* 如果不是根节点，而且解析成功，则弹出解析信息 */
        JConfLoaderInfo *info = j_stack_pop(loader->info);
        j_conf_loader_info_free(info);
        loader->top = j_stack_top(loader->info);
    }
    return ret;
}

static inline void j_conf_loader_push_line(JConfLoader * loader,
        JBufferedInputStream *
        buffered_stream, jchar * buf) {
    j_buffered_input_stream_push_line(buffered_stream, buf, -1);
    j_conf_loader_dec_line(loader);
}

/*
 * 根据value节点，载入子节点
 * 返回0表示value不是对象，或者数组，无需继续解析
 * 返回1 表示成功
 * 返回-1 表示解析失败
 */
static inline jint j_conf_loader_loads_more(JConfLoader * loader,
        JConfNode * node,
        JBufferedInputStream *
        buffered_stream, jchar * buf,
        jboolean is_root) {
    JConfNodeType type = j_conf_node_get_type(node);
    jint ret = 0;
    if (type == J_CONF_NODE_TYPE_OBJECT || type == J_CONF_NODE_TYPE_ARRAY) {
        j_conf_loader_push_line(loader, buffered_stream, buf);
        if (type == J_CONF_NODE_TYPE_OBJECT) {
            ret = j_conf_loader_loads_object
                  (loader, node, buffered_stream, FALSE);
        } else {
            ret = j_conf_loader_loads_array
                  (loader, (JConfArray *) node, buffered_stream);
        }
        ret = ret ? 1 : -1;
    }
    return ret;
}

static inline jboolean j_conf_loader_loads_object(JConfLoader * loader,
        JConfObject * object,
        JBufferedInputStream *
        buffered_stream,
        jboolean is_root) {
    JConfLoaderState state = J_CONF_LOADER_KEY;
    jchar *key = NULL;
    jchar *buf = NULL;
    JConfNode *value = NULL;
    jint i, ret;
    jboolean includeconf = FALSE;
    while ((buf =
                j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        j_conf_loader_inc_line(loader);
        i = 0;
        while (buf[i] != '\0') {
            if (j_conf_is_space(buf[i])) {
                i++;
                continue;
            }
            switch (state) {
            case J_CONF_LOADER_KEY:
                if (!is_root && buf[i] == '}') {
                    j_conf_loader_push_line(loader, buffered_stream,
                                            buf + i + 1);
                    goto OUT;
                } else if (buf[i] == ';') {
                    break;
                } else if ((ret =
                                j_conf_loader_fetch_key(loader, buf + i,
                                                        &key)) < 0) {
                    goto OUT;
                }
                i += ret - 1;
                state = J_CONF_LOADER_KEY_COLON;
                break;
            case J_CONF_LOADER_KEY_COLON:
                if (buf[i] != ':') {
                    j_conf_loader_set_errcode(loader,
                                              J_CONF_LOADER_ERR_MISSING_COLON);
                    goto OUT;
                }
                state = J_CONF_LOADER_VALUE;
                break;
            case J_CONF_LOADER_VALUE:
                if ((ret =
                            j_conf_loader_fetch_value(loader, buf + i,
                                                      &value)) < 0) {
                    goto OUT;
                }
                j_conf_object_set(object, key, value);
                if (j_strcmp0(key, "include") == 0) {
                    includeconf = TRUE;
                } else {
                    includeconf = FALSE;
                }
                j_free(key);
                key = NULL;
                i += ret - 1;
                state = J_CONF_LOADER_END;
                if ((ret = j_conf_loader_loads_more(loader, value,
                                                    buffered_stream,
                                                    buf + i + 1,
                                                    FALSE)) < 0) {
                    goto OUT;
                }
                if (includeconf) {
                    if (!j_conf_loader_include(loader, object, value)) {
                        goto OUT;
                    }
                    j_conf_object_remove(object, "include");
                }
                if (ret > 0) {
                    goto BREAK;
                }
                break;
            case J_CONF_LOADER_END:
                if (!is_root && buf[i] == '}') {
                    j_conf_loader_push_line(loader, buffered_stream,
                                            buf + i + 1);
                    goto OUT;
                } else if (buf[i] != ';') {
                    j_conf_loader_set_errcode(loader,
                                              J_CONF_LOADER_ERR_MISSING_END);
                    goto OUT;
                }
                state = J_CONF_LOADER_KEY;
                break;
            default:
                break;
            }
            i++;
            continue;
BREAK:
            break;
        }
        j_free(buf);
        if (state == J_CONF_LOADER_END) {
            state = J_CONF_LOADER_KEY;
        }
    }
    if (state == J_CONF_LOADER_KEY_COLON) {
        j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_FILE);
    } else if (state == J_CONF_LOADER_VALUE) {
        j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_VALUE);
    }

OUT:
    j_free(buf);
    j_free(key);
    return j_conf_loader_get_errcode(loader) == J_CONF_LOADER_ERR_SUCCESS;
}

static jboolean j_conf_loader_loads_array(JConfLoader * loader,
        JConfArray * array,
        JBufferedInputStream *
        buffered_stream) {
    jchar *buf;
    JConfLoaderState state = J_CONF_LOADER_ARRAY_VALUE;
    while ((buf =
                j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        j_conf_loader_inc_line(loader);
        jint i = 0, ret;
        JConfNode *value = NULL;
        while (buf[i] != '\0') {
            if (j_conf_is_space(buf[i])) {
                i++;
                continue;
            }
            switch (state) {
            case J_CONF_LOADER_ARRAY_VALUE:
                if (buf[i] == ']'
                        && j_conf_array_get_length(array) == 0) {
                    j_conf_loader_push_line(loader, buffered_stream,
                                            buf + i + 1);
                    j_free(buf);
                    return TRUE;
                } else if ((ret =j_conf_loader_fetch_value(loader, buf + i,
                                 &value)) < 0) {
                    j_conf_loader_set_errcode(loader,
                                              J_CONF_LOADER_ERR_INVALID_ARRAY_VALUE);
                    goto OUT;
                }
                i += ret - 1;
                state = J_CONF_LOADER_ARRAY_DOT;
                j_conf_array_append(array, value);
                if ((ret =j_conf_loader_loads_more(loader, value,
                                                   buffered_stream,
                                                   buf + i + 1,
                                                   FALSE)) < 0) {
                    goto OUT;
                } else if (ret > 0) {
                    goto BREAK;
                }
                break;
            case J_CONF_LOADER_ARRAY_DOT:
                if (buf[i] == ',') {
                    state = J_CONF_LOADER_ARRAY_VALUE;
                    break;
                } else if (buf[i] == ']') {
                    j_conf_loader_push_line(loader, buffered_stream,
                                            buf + i + 1);
                    j_free(buf);
                    return TRUE;
                }
                j_conf_loader_set_errcode(loader,
                                          J_CONF_LOADER_ERR_INVALID_ARRAY);
                goto OUT;
                break;
            default:
                break;
            }
            i++;
            continue;
BREAK:
            break;
        }
        j_free(buf);
    }
OUT:
    j_free(buf);
    return FALSE;
}

static inline jboolean j_conf_loader_include_path(JConfLoader * loader,
        JConfObject * root,
        const jchar * path) {
    jchar **paths = j_path_glob(path);
    if (paths == NULL) {
        j_conf_loader_set_errcode(loader,
                                  J_CONF_LOADER_ERR_INVALID_INCLUDE);
        return FALSE;
    }
    jint i = 0;
    while (paths[i] != NULL) {
        if (!j_conf_loader_loads_from_path(loader, root, paths[i], FALSE)) {
            j_strfreev(paths);
            return FALSE;
        }
        i++;
    }
    j_strfreev(paths);
    return TRUE;
}

static inline jboolean j_conf_loader_include(JConfLoader * loader,
        JConfObject * root,
        JConfNode * node) {
    JConfNodeType type = j_conf_node_get_type(node);
    if (type == J_CONF_NODE_TYPE_NULL) {
        return TRUE;
    } else if (type == J_CONF_NODE_TYPE_STRING) {
        const jchar *value = j_conf_string_get(node);
        return j_conf_loader_include_path(loader, root, value);
    } else if (type == J_CONF_NODE_TYPE_ARRAY) {
        jint i, len = j_conf_array_get_length(node);
        for (i = 0; i < len; i++) {
            JConfNode *child = j_conf_array_get(node, i);
            if (!j_conf_node_is_string(child)) {
                goto ERROR;
            }
            const jchar *value = j_conf_string_get(child);
            if (!j_conf_loader_include_path(loader, root, value)) {
                return FALSE;
            }
        }
        return TRUE;
    }
ERROR:
    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_INCLUDE);
    return FALSE;
}

jint j_conf_loader_get_errcode(JConfLoader * loader) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return 0;
    }
    return loader->top->errcode;
}

jint j_conf_loader_get_line(JConfLoader * loader) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return -1;
    }
    return loader->top->line;
}

const jchar *j_conf_loader_get_path(JConfLoader * loader) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return NULL;
    }
    return loader->top->filename;
}

static inline void j_conf_loader_set_errcode(JConfLoader * loader,
        jint errcode) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return;
    }
    loader->top->errcode = errcode;
}

static inline void j_conf_loader_inc_line(JConfLoader * loader) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return;
    }
    loader->top->line++;
}

static inline void j_conf_loader_dec_line(JConfLoader * loader) {
    if (J_UNLIKELY(loader->top == NULL)) {
        return;
    }
    loader->top->line--;
}

static inline JConfLoaderInfo *j_conf_loader_info_new(const jchar *
        filename) {
    JConfLoaderInfo *info =
        (JConfLoaderInfo *) j_malloc(sizeof(JConfLoaderInfo));
    info->filename = j_strdup(filename);
    info->errcode = 0;
    info->line = 0;
    return info;
}

static inline void j_conf_loader_info_free(JConfLoaderInfo * info) {
    j_free(info->filename);
    j_free(info);
}
