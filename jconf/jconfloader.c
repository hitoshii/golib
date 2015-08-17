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
#include <libgen.h>


struct _JConfLoader {
    JObject parent;
    JHashTable *env;
    JConfRoot *root;

    jboolean strict_var;    /* 是否允许未定义的变量 */
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
    loader->strict_var=FALSE;
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

/*
 * j_conf_loader_get:
 * @loader: JConfLoader
 * @name: the environment variable name
 *
 * Gets the value node of an environment variable
 *
 * Returns: JConfNode or NULL if not found
 */
JConfNode *j_conf_loader_get(JConfLoader * loader, const jchar * name) {
    JConfNode *node = j_hash_table_find(loader->env, name);
    return node;
}

/*
 * j_conf_loader_put_integer:
 * @loader: JConfLoader
 * @name: the name of variable
 * @integer: the value of variable
 */
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
    J_CONF_LOADER_INTEGER,        /* 下一个必须是数值 */
    J_CONF_LOADER_END,
    J_CONF_LOADER_END_INTEGER,
    J_CONF_LOADER_ARRAY_VALUE,
    J_CONF_LOADER_ARRAY_DOT,
} JConfLoaderState;

#define j_conf_is_space(c) ((c)<=32)
#define j_conf_is_end(c) (j_conf_is_space(c)||(c)==';')
#define j_conf_is_comment(c) ((c)=='#')
#define j_conf_is_key(c)    (j_ascii_isalnum(c)||(c)=='_'||(c)=='-')
#define j_conf_is_key_start(c)  (j_ascii_isalpha(c)||(c)=='_')

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
    if (!j_conf_is_key_start(buf[i])) {
        goto ERROR;
    }
    i++;
    while (buf[i] != '\0') {
        if (!j_conf_is_key(buf[i])) {
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

static inline jboolean j_strnchr(const jchar *buf, jchar c, juint len) {
    juint i=0;
    while(i<len&&buf[i]!='\0') {
        if(buf[i++]==c) {
            return TRUE;
        }
    }
    return FALSE;
}

/* 获取数字值，该函数不会返回失败 */
static inline jint j_conf_loader_fetch_digit(JConfLoader * loader,
        const jchar * buf,
        JConfNode ** value) {
    jchar *endptr1 = NULL, *endptr2 = NULL;
    if(j_strncmp0(buf, "0x", 2)==0||j_strncmp0(buf, "0X", 2)==0) {
        /* 解析十六进制数 */
        jint64 l =strtoll(buf +2, &endptr2, 16);
        *value = j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, l);
        return endptr2 - buf;
    }
    jdouble d = strtod(buf, &endptr1);
    if (!j_strnchr(buf,'.', endptr1-buf)) { /* 没有小数点，是整数 */
        jint64 l = strtoll(buf, &endptr2, 10);
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
                    j_string_append_printf(out, "%g",
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
    jchar *key = NULL;
    jint ret = j_conf_loader_fetch_key(loader, buf, &key);
    if (ret <= 0) {
        return -2;
    }
    JConfNode *node = j_conf_loader_get(loader, key);
    j_free(key);
    if (node) {
        if(j_conf_node_is_integer(node)) {
            /* 整数的值在或运算中可能会改变，因此不使用引用而是新创建一个整数节点 */
            *value=j_conf_node_new(J_CONF_NODE_TYPE_INTEGER, j_conf_integer_get(node));
        } else {
            j_conf_node_ref(node);
            *value = node;
        }
    } else if(!loader->strict_var) {
        /* 允许未定义的变量 */
        *value = j_conf_node_new(J_CONF_NODE_TYPE_NULL);
    } else {
        j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_UNKNOWN_VARIABLE);
        return -2;
    }
    return ret;
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
        return j_conf_loader_fetch_variable(loader, buf+1, value)+1;
    } else if(j_ascii_isalpha(buf[0])) {
        return j_conf_loader_fetch_variable(loader, buf, value);
    }
    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_VALUE);
    return -1;
}

static inline void j_conf_loader_clear(JConfLoader *loader) {
    j_stack_clear(loader->info, (JDestroyNotify) j_conf_loader_info_free);
    j_conf_node_unref((JConfNode*)j_conf_loader_get_root(loader));
    loader->root=j_conf_root_new();
    loader->top=NULL;
}


void j_conf_loader_allow_unknown_variable(JConfLoader *loader, jboolean allow) {
    loader->strict_var=!allow;
}

/*
 * 载入一个配置文件，这会释放原来的JConfRoot，如果要保留原来的JConfRoot，使用j_conf_node_ref()
 */
jboolean j_conf_loader_loads(JConfLoader * loader, const jchar * path) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    jchar *basename=j_path_basename(path);
    jchar *dirname=j_path_dirname(path);
    chdir(dirname);
    j_conf_loader_clear(loader);
    jboolean ret = j_conf_loader_loads_from_path(loader, (JConfObject *)
                   j_conf_loader_get_root(loader),
                   basename, TRUE);

    chdir(cwd);
    j_free(basename);
    j_free(dirname);
    return ret;
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
    JConfNode *integer=NULL;
    jint i, ret;
    jboolean includeconf = FALSE;
    while ((buf =
                j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        j_conf_loader_inc_line(loader);
        i = 0;
        while (buf[i] != '\0') {
            if (j_conf_is_comment(buf[i])) {
                goto BREAK;
            } else if (j_conf_is_space(buf[i])) {
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
                } else if ((ret = j_conf_loader_fetch_key(loader, buf + i,
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
                if ((ret =j_conf_loader_fetch_value(loader, buf + i,
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
                if(j_conf_node_is_integer(value)) {
                    state=J_CONF_LOADER_END_INTEGER;
                    ret=0;
                } else if ((ret = j_conf_loader_loads_more(loader, value,
                                  buffered_stream,
                                  buf + i + 1,
                                  FALSE)) < 0) {
                    /* 读取object或者array失败 */
                    goto OUT;
                } else {
                    state = J_CONF_LOADER_END;
                }
                if (includeconf) {
                    if (!j_conf_loader_include(loader, object, value)) {
                        goto OUT;
                    }
                    j_conf_object_remove(object, "include");
                }
                if (ret > 0) {
                    /* 读取object或者array成功 */
                    goto BREAK;
                }
                break;
            case J_CONF_LOADER_INTEGER:
                if((ret=j_conf_loader_fetch_value(loader, buf+i, &integer))<=0) {
                    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_INTEGER);
                    goto OUT;
                } else if(!j_conf_node_is_integer(integer)) {
                    j_conf_node_unref(integer);
                    j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_INVALID_INTEGER);
                    goto OUT;
                }
                i+=ret-1;
                j_conf_integer_set(value, j_conf_integer_get(value)|j_conf_integer_get(integer));
                j_conf_node_unref(integer);
                state=J_CONF_LOADER_END_INTEGER;
                break;
            case J_CONF_LOADER_END_INTEGER:
                if(buf[i]=='|') {
                    state=J_CONF_LOADER_INTEGER;
                    break;
                }
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

        if (state == J_CONF_LOADER_END||state==J_CONF_LOADER_END_INTEGER) {
            state = J_CONF_LOADER_KEY;
        } else if(state==J_CONF_LOADER_KEY_COLON) {
            j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_MISSING_COLON);
            goto OUT;
        } else if(state==J_CONF_LOADER_VALUE) {
            j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_MISSING_VALUE);
            goto OUT;
        } else if(state==J_CONF_LOADER_INTEGER) {
            j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_MISSING_INTEGER);
            goto OUT;
        }
        j_free(buf);
    }
    if(state==J_CONF_LOADER_KEY_COLON) {
        j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_MISSING_COLON);
    } else if(state==J_CONF_LOADER_VALUE) {
        j_conf_loader_set_errcode(loader, J_CONF_LOADER_ERR_MISSING_VALUE);
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
    while ((buf =j_buffered_input_stream_readline(buffered_stream)) != NULL) {
        j_conf_loader_inc_line(loader);
        jint i = 0, ret;
        JConfNode *value = NULL;
        while (buf[i] != '\0') {
            if(j_conf_is_comment(buf[i])) {
                goto BREAK;
            } else if (j_conf_is_space(buf[i])) {
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


char *j_conf_loader_build_error_message(JConfLoader *loader) {
    if(J_UNLIKELY(loader->top==NULL)) {
        return NULL;
    }
    jchar *msg=NULL;
    JConfLoaderInfo *info=loader->top;
    switch(info->errcode) {
    case J_CONF_LOADER_ERR_SUCCESS:
        msg=j_strdup_printf("%s Success", info->filename);
        break;
    case J_CONF_LOADER_ERR_INVALID_FILE:
        msg = j_strdup_printf("Fail to open %s", info->filename);
        break;
    case J_CONF_LOADER_ERR_INVALID_KEY:
        msg = j_strdup_printf("%s: %u - invalid key", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_MISSING_COLON:
        msg = j_strdup_printf("%s: %u - : is required", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_INVALID_VALUE:
        msg = j_strdup_printf("%s: %u - invalid value", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_MISSING_VALUE:
        msg=j_strdup_printf("%s: %u - value is missing", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_INVALID_STRING:
        msg = j_strdup_printf("%s: %u - malformed string", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_MISSING_END:
        msg = j_strdup_printf("%s: %u - ; or newline is required", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_INVALID_ARRAY:
        msg = j_strdup_printf("%s: %u - malformed array", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_INVALID_INCLUDE:
        msg = j_strdup_printf("%s: %u - invalid include path", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_UNKNOWN_VARIABLE:
        msg=j_strdup_printf("%s: %u - uknown variable", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_INVALID_INTEGER:
        msg=j_strdup_printf("%s: %u - not a valid integer value", info->filename, info->line);
        break;
    case J_CONF_LOADER_ERR_MISSING_INTEGER:
        msg=j_strdup_printf("%s: %u - integer value is required", info->filename, info->line);
        break;
    default:
        msg=j_strdup_printf("%s: %u - unknown error", info->filename, info->line);
        break;
    }
    return msg;
}
