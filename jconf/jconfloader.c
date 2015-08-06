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

    /* 错误信息 */
    jint errcode;
    juint line;
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
    loader->errcode = 0;
    loader->line = 0;
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
    J_CONF_LOADER_KEY = 0,      /* 下一个应该是一个键值 */
    J_CONF_LOADER_KEY_COLON,    /* 下一个应该是冒号 */
    J_CONF_LOADER_VALUE,
    J_CONF_LOADER_END,
    J_CONF_LOADER_ARRAY_VALUE,
    J_CONF_LOADER_ARRAY_DOT,
} JConfLoaderState;

#define j_conf_is_space(c) ((c)<=32)
#define j_conf_is_end(c) (j_conf_is_space(c)||(c)==';')


static jchar *j_conf_errors[] = {
    "success",
    "invalid key",
    "invalid value",
    "invalid string",
    ": required",
    "; required"
};

#define ERROR_UNKNOWN 0
#define ERROR_INVALID_KEY 1
#define ERROR_INVALID_VALUE 2
#define ERROR_INVALID_STRING 3
#define ERROR_COLON_MISSING 4
#define ERROR_PUNC_MISSING 5

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
                                           const jchar * buf, jchar ** key)
{
    jboolean quote = FALSE;
    jint i = 0;
    if (buf[0] == '\"') {
        quote = TRUE;
        i++;
    }
    if (!(j_ascii_isalpha(buf[i]) || buf[i] == '_')) {
        loader->errcode = ERROR_INVALID_KEY;
        return -1;
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
            loader->errcode = ERROR_INVALID_KEY;
            return -1;
        }
        *key = j_strndup(buf + 1, i - 1);
        return i + 1;
    }
    *key = j_strndup(buf, i);
    return i;
}

static inline jint j_conf_loader_fetch_digit(JConfLoader * loader,
                                             const jchar * buf,
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

/*
 * 解析四个字节表示的16进制数
 */
static inline juint32 parse_hex4(const jchar * str)
{
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


static inline jint j_conf_loader_fetch_string(JConfLoader * loader,
                                              const jchar * buf,
                                              JConfNode ** value)
{
    static const juchar firstByteMark[7] = {
        0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
    };
    if (J_UNLIKELY(buf[0] != '\"')) {
        loader->errcode = ERROR_INVALID_STRING;
        return -1;
    }
    jchar *ptr = (jchar *) (buf + 1);
    jint len = 0;
    while (*ptr != '\"') {
        if (*ptr == '\0') {
            loader->errcode = ERROR_INVALID_STRING;
            return -1;
        } else if (*ptr++ == '\\') {
            ptr++;
        }
        len++;
    }
    const jchar *start = buf++;
    jchar *out = (jchar *) malloc(sizeof(jchar) * (len + 1));
    ptr = out;
    juint32 uc, uc2;
    while (*buf != '\"') {
        if (*buf != '\\') {
            *ptr++ = *buf++;
        } else {
            buf++;
            switch (*buf) {
            case 'b':
                *ptr++ = '\b';
                break;
            case 'f':
                *ptr++ = '\f';
                break;
            case 'n':
                *ptr++ = '\n';
                break;
            case 'r':
                *ptr++ = '\r';
                break;
            case 't':
                *ptr++ = '\t';
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
                ptr += len;
                switch (len) {
                case 4:
                    *--ptr = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 3:
                    *--ptr = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 2:
                    *--ptr = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 1:
                    *--ptr = (uc | firstByteMark[len]);
                }
                ptr += len;
                break;
            default:
                *ptr++ = *buf;
            }
            buf++;
        }
    }
    *ptr = '\0';
    *value = j_conf_node_new(J_CONF_NODE_TYPE_STRING, out);
    j_free(out);
    return buf + 1 - start;
  ERROR:
    j_free(out);
    loader->errcode = ERROR_INVALID_STRING;
    return -1;
}

static inline jint j_conf_loader_fetch_value(JConfLoader * loader,
                                             const jchar * buf,
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
        return j_conf_loader_fetch_digit(loader, buf, value);
    } else if (buf[0] == '\"') {
        return j_conf_loader_fetch_string(loader, buf, value);
    } else if (buf[0] == '{') {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_OBJECT);
        return 1;
    } else if (buf[0] == '[') {
        *value = j_conf_node_new(J_CONF_NODE_TYPE_ARRAY);
        return 1;
    }
    loader->errcode = ERROR_INVALID_VALUE;
    return -1;
}

/*
 * 成功返回1
 * 失败返回小于等于0的错误码
 */
static jboolean j_conf_loader_loads_object(JConfLoader * loader,
                                           JConfObject * root,
                                           JBufferedInputStream *
                                           input_stream, jboolean is_root);
static jboolean j_conf_loader_loads_array(JConfLoader * loader,
                                          JConfArray * array,
                                          JBufferedInputStream *
                                          buffered_stream);

jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path)
{
    JFileInputStream *input_stream = j_file_input_stream_open(path);
    if (input_stream == NULL) {
        return FALSE;
    }
    JBufferedInputStream *buffered_stream =
        j_buffered_input_stream_new((JInputStream *) input_stream);
    jboolean ret = j_conf_loader_loads_object(loader,
                                              (JConfObject *)
                                              j_conf_loader_get_root
                                              (loader),
                                              buffered_stream, TRUE);
    j_file_input_stream_unref(input_stream);
    j_buffered_input_stream_unref(buffered_stream);

    if (!ret) {
        j_fprintf(stderr, "%s:%lu - %s\n", path, loader->line,
                  j_conf_errors[loader->errcode]);
    }
    return ret;
}

static inline void j_conf_loader_push_line(JConfLoader * loader,
                                           JBufferedInputStream *
                                           buffered_stream, jchar * buf)
{
    j_buffered_input_stream_push_line(buffered_stream, buf, -1);
    loader->line--;
}

static inline jint j_conf_loader_loads_more(JConfLoader * loader,
                                            JConfNode * node,
                                            JBufferedInputStream *
                                            buffered_stream, jchar * buf,
                                            jboolean is_root)
{
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

static jboolean j_conf_loader_loads_object(JConfLoader * loader,
                                           JConfObject * root,
                                           JBufferedInputStream *
                                           buffered_stream,
                                           jboolean is_root)
{
    JConfLoaderState state = J_CONF_LOADER_KEY;
    jchar *key = NULL, *buf = NULL;
    JConfNode *value = NULL;
    jint i, ret;
    while ((buf =
            j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        loader->line++;
        i = 0;
        while (buf[i] != '\0') {
            if (!j_conf_is_space(buf[i])) {
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
                        loader->errcode = ERROR_COLON_MISSING;
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
                    j_conf_object_set_take(root, key, value);
                    key = NULL;
                    i += ret - 1;
                    state = J_CONF_LOADER_END;
                    if ((ret =
                         j_conf_loader_loads_more(loader, value,
                                                  buffered_stream,
                                                  buf + i + 1,
                                                  FALSE)) < 0) {
                        goto OUT;
                    } else if (ret > 0) {
                        goto BREAK;
                    }
                    break;
                case J_CONF_LOADER_END:
                    if (!is_root && buf[i] == '}') {
                        j_conf_loader_push_line(loader, buffered_stream,
                                                buf + i + 1);
                        goto OUT;
                    } else if (buf[i] != ';') {
                        loader->errcode = ERROR_PUNC_MISSING;
                        goto OUT;
                    }
                    state = J_CONF_LOADER_KEY;
                    break;
                }
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

  OUT:
    j_free(buf);
    j_free(key);
    return loader->errcode == 0;
}

static jboolean j_conf_loader_loads_array(JConfLoader * loader,
                                          JConfArray * array,
                                          JBufferedInputStream *
                                          buffered_stream)
{
    jchar *buf;
    JConfLoaderState state = J_CONF_LOADER_ARRAY_VALUE;
    while ((buf =
            j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        loader->line++;
        jint i = 0, ret;
        JConfNode *value = NULL;
        while (buf[i] != '\0') {
            if (!j_conf_is_space(buf[i])) {
                switch (state) {
                case J_CONF_LOADER_ARRAY_VALUE:
                    if (buf[i] == ']'
                        && j_conf_array_get_length(array) == 0) {
                        j_conf_loader_push_line(loader, buffered_stream,
                                                buf + i + 1);
                        j_free(buf);
                        return TRUE;
                    } else
                        if ((ret =
                             j_conf_loader_fetch_value(loader, buf + i,
                                                       &value)) < 0) {
                        goto OUT;
                    }
                    i += ret - 1;
                    state = J_CONF_LOADER_ARRAY_DOT;
                    j_conf_array_append(array, value);
                    if ((ret =
                         j_conf_loader_loads_more(loader, value,
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
                    goto OUT;
                    break;
                }
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
    loader->errcode = ERROR_INVALID_VALUE;
    return FALSE;
}
