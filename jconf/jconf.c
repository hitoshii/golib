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
#include "jconf.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>

#define ERROR_MAX    2048

static char g_strerror[ERROR_MAX];
static int j_errno = J_CONF_SUCCESS;

int j_conf_errno(void)
{
    return j_errno;
}

const char *j_conf_strerror(void)
{
    return g_strerror;
}

static inline void j_conf_set_error(int errcode, const char *fmt, ...)
{
    j_errno = errcode;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_strerror, sizeof(g_strerror) / sizeof(char), fmt, ap);
    va_end(ap);
}

/*
 * 设置错误码，出错说明中添加文件名和行号
 */
static inline void j_conf_root_set_error(JConfRoot * root, int errcode,
                                         const char *fmt, ...)
{
    j_errno = errcode;
    va_list ap;
    va_start(ap, fmt);
    int n =
        snprintf(g_strerror, sizeof(g_strerror) / sizeof(char), "[%s:%u] ",
                 j_conf_root_get_name(root),
                 j_conf_root_get_line(root));
    vsnprintf(g_strerror + n, sizeof(g_strerror) / sizeof(char) - n, fmt,
              ap);
    va_end(ap);
}

/*
 * 跳过没有用的字符，包括注释
 */
static inline const char *j_conf_skip(const char *data);

/*
 * 解析一个指令名，类似C语言中的变量，字母开头
 */
static inline const char *j_conf_name_parse(char **name, const char *data);
/*
 * 解析一个节点，字符串、整数、浮点数、对象或者数组
 */
static inline const char *j_conf_node_parse(JConfRoot * root,
                                            JConfNode ** node,
                                            const char *data);
/*
 * 解析字符串，以双引号包裹，支持unicode解析
 */
static inline const char *j_conf_string_parse(JConfRoot * root, char **ret,
                                              const char *data);
/*
 * 解析数字，整型和浮点数，支持科学计数法
 */
static inline const char *j_conf_number_parse(JConfRoot * root,
                                              JConfNode ** node,
                                              const char *data);
/*
 * 解析一个整数
 */
static inline const char *j_conf_int_parse(int64_t * integer,
                                           const char *data);
/*
 * 解析一个数组
 */
static inline const char *j_conf_array_parse(JConfRoot * root,
                                             JConfNode ** node,
                                             const char *data);
/*
 * 解析一个对象
 */
static inline const char *j_conf_object_parse(JConfRoot * root,
                                              JConfNode ** node,
                                              const char *data);
/*
 * 从一个新文件中解析配置文件
 */
static inline int j_conf_root_parse_file(JConfRoot * root,
                                         const char *data);
/*
 * 从data中解析配置
 */
static inline int j_conf_root_parse(JConfRoot * root, const char *data);
/*
 * 解析被包含的配置文件
 */
static inline int j_conf_include(JConfRoot * root, JConfNode * include);
static inline int j_conf_include_array(JConfRoot * root,
                                       JConfNode * include);
static inline int j_conf_include_string(JConfRoot * root,
                                        JConfNode * include);
/*
 * 将一个文件内容影射为内存空间
 * 并添加\0结尾
 */
static inline char *j_conf_map_file(const char *path, unsigned int *len);
/*
 * 取消一个内存空间对文件的影射
 */
static inline void j_conf_unmap_data(char *data, unsigned int len);


static inline char *j_conf_map_file(const char *path, unsigned int *len)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        j_conf_set_error(J_CONF_ERROR_FILE, "Unable to open %s: %s", path,
                         strerror(errno));
        return NULL;
    }
    struct stat sbuf;
    if (fstat(fd, &sbuf) < 0) {
        close(fd);
        j_conf_set_error(J_CONF_ERROR_FILE,
                         "Unable to get the length of %s", path,
                         strerror(errno));
        return NULL;
    }
    unsigned int size = sbuf.st_size;
    char *data =
        (char *) mmap(NULL, size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE,
                      fd,
                      0);
    close(fd);
    if (data == MAP_FAILED) {   /* MAP_FAILED = (void*)-1 */
        j_conf_set_error(J_CONF_ERROR_FILE, "mmap error: %s",
                         strerror(errno));
        return NULL;
    }
    data[size++] = '\0';
    *len = size;
    return data;
}

static inline void j_conf_unmap_data(char *data, unsigned int len)
{
    munmap(data, len);
}

/*
 * 从一个文件中载入配置
 */
JConfRoot *j_conf_load_from_file(const char *path)
{
    unsigned int len;
    char *data = j_conf_map_file(path, &len);
    if (data == NULL) {
        return NULL;
    }
    JConfRoot *root = j_conf_root_new(path);
    if (!j_conf_root_parse(root, data)) {
        j_conf_root_free(root);
        root = NULL;
    }
    j_conf_unmap_data(data, len);
    return root;
}

/*
 * 载入配置文件
 * 成功返回1，失败返回0
 */
int j_conf_root_load(JConfRoot * root)
{
    unsigned int len;
    char *data = j_conf_map_file(j_conf_root_get_name(root), &len);
    if (data == NULL) {
        return 0;
    }
    int ret = j_conf_root_parse(root, data);
    j_conf_unmap_data(data, len);
    return ret;
}

/*
 * 载入额外的配置文件
 */
static inline int j_conf_root_parse_file(JConfRoot * root,
                                         const char *path)
{
    unsigned int len;
    char *data = j_conf_map_file(path, &len);
    if (data == NULL) {
        return 0;
    }
    j_conf_root_push(root, path);
    int ret = j_conf_root_parse(root, data);
    j_conf_unmap_data(data, len);
    return ret;
}

/*
 * 判断一个字符是否为指令结束符号
 */
static inline int j_conf_root_isbreak(JConfRoot * root, char c)
{
    if (c == ';') {
        return 1;
    } else if (c == '\n') {
        j_conf_root_line_add(root);
        return 1;
    }
    return 0;
}

static inline int j_conf_root_parse(JConfRoot * root, const char *data)
{
    j_conf_set_error(J_CONF_SUCCESS, "Success");
    data = j_conf_skip(data);
    while (*data) {
        if (j_conf_root_isbreak(root, *data)) {
            data = j_conf_skip(data + 1);
            continue;
        } else if (!isalpha(*data)) {
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Ascii letter is required", *data, data);
            return 0;
        }
        char *name = NULL;
        JConfNode *child = NULL;
        if ((data = j_conf_skip(j_conf_name_parse(&name, data))) == NULL) {
            j_free(name);
            return 0;
        }
        if (j_conf_root_isbreak(root, *data) || *data == '\0') {
            child = j_conf_node_create_null();
        } else if (*data == ':') {
            if ((data =
                 j_conf_skip(j_conf_node_parse(root, &child, data + 1))) ==
                NULL) {
                j_free(name);
                return 0;
            }
        } else {
            j_free(name);
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Break or colon is required", *data,
                                  data);
            return 0;
        }
        j_conf_node_set_name_take(child, name);
        if (strcmp(name, J_CONF_INCLUDE) == 0) {    /* 读取子配置文件 */
            if (!j_conf_include(root, child)) {
                return 0;
            }
        } else {
            j_conf_root_append(root, child);
        }
        data = j_conf_skip(data);
        if (!j_conf_root_isbreak(root, *data) && *data != '\0') {
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Break is required", *data, data);
            return 0;
        }
        if (*data != '\0') {
            data = j_conf_skip(data + 1);
        }
    }
    return 1;
}

static inline int j_conf_include_array(JConfRoot * root,
                                       JConfNode * include)
{
    JList *children = j_conf_node_get_children(include);
    while (children) {
        JConfNode *node = (JConfNode *) j_list_data(children);
        if (j_conf_node_is_string(node)) {
            if (!j_conf_include_string(root, node)) {
                return 0;
            }
        } else {
            return 0;
        }
        children = j_list_next(children);
    }
    return 1;
}

static inline int j_conf_include_string(JConfRoot * root,
                                        JConfNode * include)
{
    const char *path = j_conf_string_get(include);
    return j_conf_root_parse_file(root, path);
}

static inline int j_conf_include(JConfRoot * root, JConfNode * include)
{
    int ret = 0;
    if (j_conf_node_is_string(include)) {
        ret = j_conf_include_string(root, include);
    } else if (j_conf_node_is_array(include)) {
        ret = j_conf_include_array(root, include);
    }
    j_conf_node_free(include);
    return ret;
}

static inline const char *j_conf_name_parse(char **name, const char *data)
{
    const char *start = data++;
    while (isalnum(*data)) {
        data++;
    }
    *name = j_strndup(start, data - start);
    return data;
}

static inline const char *j_conf_node_parse(JConfRoot * root,
                                            JConfNode ** node,
                                            const char *data)
{
    data = j_conf_skip(data);
    if (*data == '\"') {        /* 字符串 */
        char *string = NULL;
        if ((data = j_conf_string_parse(root, &string, data)) == NULL) {
            return NULL;
        }
        *node = j_conf_node_create_string_take(string);
    } else if (*data == '-' || isdigit(*data)) {    /* 数字  */
        data = j_conf_number_parse(root, node, data);
    } else if (strncmp(data, "true", 4) == 0) {
        *node = j_conf_node_create_true();
        data += 4;
    } else if (strncmp(data, "false", 5) == 0) {
        *node = j_conf_node_create_false();
        data += 5;
    } else if (*data == '[') {  /* 数组 */
        data = j_conf_array_parse(root, node, data);
    } else if (*data == '{') {  /* object */
        data = j_conf_object_parse(root, node, data);
    } else {
        j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                              "Invalid character \'%c\' near \"%.10s\".",
                              *data, data);
        return NULL;
    }
    return data;
}

static inline const char *j_conf_object_parse(JConfRoot * root,
                                              JConfNode ** node,
                                              const char *data)
{
    data = j_conf_skip(data + 1);

    *node = j_conf_node_create_object();
    while (*data) {
        char *name = NULL;
        JConfNode *child = NULL;
        if (j_conf_root_isbreak(root, *data)) {
            data = j_conf_skip(data + 1);
            continue;
        } else if (*data == '}') {
            return data + 1;
        } else if (!isalpha(*data)) {
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Ascii letter is required", *data, data);
            goto ERROR;
        }
        data = j_conf_skip(j_conf_name_parse(&name, data));
        if (j_conf_root_isbreak(root, *data)) {
            child = j_conf_node_create_null();
        } else if (*data == ':') {
            if ((data = j_conf_node_parse(root, &child, data + 1)) == NULL) {
                free(name);
                goto ERROR;
            }
        } else {
            free(name);
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Break or colon is required", *data,
                                  data);
            goto ERROR;
        }
        j_conf_node_set_name_take(child, name);
        j_conf_node_append_child(*node, child);
        data = j_conf_skip(data);
        if (j_conf_root_isbreak(root, *data)) {
            data = j_conf_skip(data + 1);
        } else if (*data == '}') {
            return data + 1;
        } else {
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Break is required", *data, data);
            goto ERROR;
        }
    }
  ERROR:
    j_conf_node_free(*node);
    return NULL;
}

static inline const char *j_conf_array_parse(JConfRoot * root,
                                             JConfNode ** node,
                                             const char *data)
{
    *node = j_conf_node_create_array();
    data = j_conf_skip(data + 1);
    while (*data) {
        if (*data == ']') {
            return data + 1;
        } else if (*data == '\n') {
            data = j_conf_skip(data + 1);
            continue;
        }
        JConfNode *child = NULL;
        if ((data = j_conf_node_parse(root, &child, data)) == NULL) {
            goto ERROR;
        }
        j_conf_node_append_child(*node, child);

        data = j_conf_skip(data);
        while (*data == '\n') {
            data = j_conf_skip(data + 1);
        }
        if (*data == ',') {
            data = j_conf_skip(data + 1);
        } else if (*data == ']') {
            return data + 1;
        } else if (*data != '\n') {
            j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                                  "Invalid character \'%c\' near \"%.10s\". "
                                  "Break is required", *data, data);
            goto ERROR;
        }
    }
  ERROR:
    j_conf_node_free(*node);
    return NULL;
}

/*
 * 解析四个字节表示的16进制数
 */
static inline uint32_t parse_hex4(const char *str)
{
    unsigned int h = 0;
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

static const unsigned char firstByteMark[7] =
    { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static inline const char *j_conf_string_parse(JConfRoot * root, char **ret,
                                              const char *data)
{
    const char *origin = data;
    if (*data != '\"') {
        return NULL;
    }
    char *ptr = (char *) ++data;
    int len = 0;
    while (*ptr != '\"') {
        if (*ptr == '\0') {
            return NULL;
        } else if (*ptr++ == '\\') {
            ptr++;              /* Skip escaped quotes. */
        }
        len++;
    }
    char *out = (char *) malloc(sizeof(char) * len);    /* allocates enough memory for string */
    ptr = out;
    uint32_t uc, uc2;
    while (*data != '\"') {
        if (*data != '\\') {
            *ptr++ = *data++;
        } else {
            data++;
            switch (*data) {
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
            case 'u':          /* UNICODE */
                uc = parse_hex4(data + 1);
                data += 4;      /* gets the unicode char */
                if (uc == 0 || (uc >= 0xDC00 && uc <= 0xDFFF)) {
                    goto ERROR;
                } else if (uc >= 0xD800 && uc <= 0xDBFF) {
                    if (data[1] != '\\' || data[2] != 'u') {
                        goto ERROR; /* second-half is missing */
                    }
                    uc2 = parse_hex4(data + 3);
                    data += 6;
                    if (uc2 < 0xDC00 || uc2 > 0xDFFF) {
                        goto ERROR; /* invalid second-half */
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
                data++;
                break;
            default:
                *ptr++ = *data++;
            }
        }
    }
    *ptr = '\0';
    *ret = j_conf_root_assign(root, out);
    return data + 1;
  ERROR:
    free(out);
    j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                          "Invalid string near \'%.15s\'. ", origin);
    return NULL;
}

/*
 * 实现pow10，避免需要加入math.h
 */
static inline double pow_10(int p)
{
    double sum = 1;
    while (p > 0) {
        sum *= 10;
        p--;
    }
    while (p < 0) {
        sum /= 10;
        p++;
    }
    return sum;
}


/*
 * 解析一个整数
 */
static inline const char *j_conf_int_parse(int64_t * integer,
                                           const char *data)
{
    int sign = 1;
    if (*data == '-') {
        sign = -1;
        data++;
    } else if (*data == '+') {
        data++;
    }
    const char *ptr = data;
    int len = 0;
    while (1) {
        if (*ptr == '\0') {
            return NULL;
        } else if (isdigit(*ptr)) {
            ptr++;
            len++;
        } else {
            break;
        }
    }
    int64_t sum = 0;
    while (data < ptr) {
        sum += (*data++ - '0') * pow_10(--len);
    }
    *integer = sign * sum;
    return ptr;
}

static inline const char *j_conf_number_parse(JConfRoot * root,
                                              JConfNode ** node,
                                              const char *data)
{
    const char *origin = data;
    int sign = 1;
    if (*data == '-') {
        sign = -1;
        data++;
    }
    const char *start = data;
    const char *dot = NULL;
    const char *e = NULL;
    while (*data) {
        if (*data == '.') {
            if (dot != NULL) {  /* 不能出现两个小数点  */
                return NULL;
            }
            dot = data;
        } else if (*data == 'e' || *data == 'E') {
            e = data + 1;
            break;
        } else if (!isdigit(*data)) {
            break;
        }
        data++;
    }
    if (*data == '\0' || data <= start) {
        return NULL;
    }
    if (dot == NULL) {
        dot = data;
    }
    int len = dot - start - 1;
    int64_t integer = 0;
    while (start < dot) {
        integer += (*start++ - '0') * pow_10(len--);
    }
    int64_t floating = 0;
    if (dot < data - 1) {       /* float */
        start = dot + 1;
        len = data - start;
        int j = len - 1;
        while (start < data) {
            floating += (*start - '0') * pow_10(j);
            j--;
            start++;
        }
    }
    double ret = sign * ((double) integer +
                         (double) floating / pow_10(len));
    if (e) {
        int64_t inte;
        if ((data = j_conf_int_parse(&inte, e)) == NULL) {
            return NULL;
        }
        double doublee = pow_10(inte);
        ret *= doublee;
    }
    if ((int64_t) ret != ret) {
        *node = j_conf_node_create_float(ret);
    } else {
        *node = j_conf_node_create_int((int64_t) ret);
    }

    if (data == NULL) {
        j_conf_root_set_error(root, J_CONF_ERROR_MALFORMED,
                              "Invalid number near \'%.10s\'. ", origin);
    }
    return data;
}

static inline const char *j_conf_skip(const char *data)
{
    if (data == NULL) {
        return NULL;
    }
    int comment = 0;
    while (*data) {
        if (comment) {
            if (*data == '\n') {
                comment = 0;
                return data;
            }
        } else {
            if (*data == '#') {
                comment = 1;
            } else if (*data > ' ' || *data == '\n') {
                break;
            }
        }
        data++;
    }
    return data;
}
