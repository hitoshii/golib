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

#include "parser.h"
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <jlib/jlib.h>
#include <jio/jfile.h>


typedef struct {
    char *name;
    char *value;
} JConfVar;
#define j_conf_var_get_name(env)    ((var)->name)
#define j_conf_var_get_value(env)   ((var)->value)

static inline JConfVar *j_conf_var_new(const char *name, const char *value)
{
    JConfVar *var = (JConfVar *) j_malloc(sizeof(JConfVar));
    var->name = j_strdup(name);
    var->value = j_strdup(value);
    return var;
}

static inline void j_conf_var_free(JConfVar * var)
{
    j_free(var->name);
    j_free(var->value);
    j_free(var);
}


/************************************************************************/

struct _JConfParser {
    JList *vars;
    JList *envs;
    JConfNode *root;            /* a root group */
    /* the root node is a group whose name is empty(NULL) */

    JList *filepaths;           /* the list of config files */
};

JConfNode *j_conf_parser_get_root(JConfParser * p)
{
    return p->root;
}


JConfParser *j_conf_parser_new()
{
    JConfParser *parser = (JConfParser *) j_malloc(sizeof(JConfParser));
    parser->vars = NULL;
    parser->envs = NULL;
    parser->root = j_conf_node_new(J_CONF_NODE_SCOPE, NULL);
    parser->filepaths = NULL;
    return parser;
}

/*
 * Checks to see if the path has been already parsed by this JConfParser
 * This function is used to guarantee a file will not be
 * loaded more than one times
 */
static inline int j_conf_parser_is_parsed(JConfParser * p,
                                          const char *path)
{
    char *realpath = j_path_realpath(path);
    if (realpath == NULL) {
        return 0;
    }
    JList *ptr = p->filepaths;
    while (ptr) {
        if (j_strcmp0(realpath, (const char *) j_list_data(ptr)) == 0) {
            j_free(realpath);
            return 1;
        }
        ptr = j_list_next(ptr);
    }
    p->filepaths = j_list_append(p->filepaths, realpath);
    return 0;
}

/*
 * Frees JConfParser
 */
void j_conf_parser_free(JConfParser * parser)
{
    j_list_free_full(parser->vars, (JListDestroy) j_conf_var_free);
    j_list_free_full(parser->envs, (JListDestroy) free);
    j_list_free_full(parser->filepaths, (JListDestroy) free);
    j_conf_node_free(parser->root);
    j_free(parser);
}

void j_conf_parser_add_variable(JConfParser * parser, const char *str)
{
    char **strv = j_strsplit_c(str, '=', 2);
    if (j_strv_length(strv) == 2) {
        JConfVar *var = j_conf_var_new(strv[0], strv[1]);
        parser->vars = j_list_append(parser->vars, var);
    }

    j_strfreev(strv);
}

/*
 * Adds a enveriment path
 */
void j_conf_parser_add_env(JConfParser * parser, const char *str)
{
    parser->envs = j_list_append(parser->envs, j_strdup(str));
}


JList *j_conf_parser_get_envs(JConfParser * parser)
{
    return parser->envs;
}

/*
 */
static inline char **j_conf_parser_get_path(JConfParser * parser,
                                            const char *path)
{
    if (j_path_is_absolute(path)) {
        return j_path_glob(path);
    }
    char *joined = NULL;
    JList *env = j_conf_parser_get_envs(parser);
    while (env) {
        const char *env_path = j_list_data(env);
        joined = j_path_join(env_path, path);
        char **matched = j_path_glob(joined);
        j_free(joined);
        if (matched) {
            return matched;
        }
        env = j_list_next(env);
    }
    return NULL;
}

static inline char *j_conf_parser_preprocess(JConfParser * parser,
                                             char *data)
{
    JList *var = parser->vars;
    char *new = data;
    char buf[1024];
    while (var) {
        JConfVar *v = (JConfVar *) j_list_data(var);
        snprintf(buf, sizeof(buf) / sizeof(char), "{%s}", v->name);
        new = j_str_replace(data, buf, v->value);
        data = new;
        var = j_list_next(var);
    }
    return new;
}

static inline int j_conf_parser_error(char **errstr, const char *fmt, ...)
{
    if (errstr) {
        va_list vl;
        va_start(vl, fmt);
        *errstr = j_strdup_vprintf(fmt, vl);
        va_end(vl);
    }
    return 0;
}


typedef enum {
    J_CONF_PARSER_STATE_NEW,
    J_CONF_PARSER_STATE_DIRECTIVE_NAME,
    J_CONF_PARSER_STATE_DIRECTIVE_NAME_END,
    J_CONF_PARSER_STATE_DIRECTIVE_VALUE,
    J_CONF_PARSER_STATE_SCOPE,
    J_CONF_PARSER_STATE_SCOPE_START,
    J_CONF_PARSER_STATE_SCOPE_START_NAME,
    J_CONF_PARSER_STATE_SCOPE_START_NAME_END,
    J_CONF_PARSER_STATE_SCOPE_START_VALUE,
    J_CONF_PARSER_STATE_SCOPE_START_END,
    J_CONF_PARSER_STATE_SCOPE_END,
    J_CONF_PARSER_STATE_SCOPE_END_NAME,
    J_CONF_PARSER_STATE_SCOPE_END_NAME_END,
    J_CONF_PARSER_STATE_SCOPE_END_END,
} JConfParserState;


#define j_isspace(c)    (isspace(c)||c=='\t')
#define j_isname(c)     (isalnum(c)||c=='_')
#define j_isscope(c)    (c=='<')
#define j_isclose(c)    (c=='>')
#define j_isend(c)      (c=='/')
/*
 * Parses a conf file,
 * If error occurs, errstr will be set the error string (optional), and returns 0
 * Returns 1 on success
 */
int j_conf_parser_parse(JConfParser * parser, const char *filepath,
                        char **errstr)
{
    if (j_conf_parser_is_parsed(parser, filepath)) {
        return 1;
    }
    int ret = 0;
    char *data = j_file_readall(filepath);
    if (data == NULL) {
        return j_conf_parser_error(errstr, "%s", strerror(errno));
    }
    data = j_conf_parser_preprocess(parser, data);
    char **lines = j_strsplit_c(data, '\n', -1);
    j_free(data);

    char **line = lines;
    char *name = NULL;
    char *value = NULL;
    unsigned int ln = 0;        /* line number */
    JStack *scopes = j_stack_new();
    j_stack_push(scopes, j_conf_parser_get_root(parser));
    while (*line) {
        ln++;
        char *comment = strchr(*line, '#');
        if (comment) {
            *comment = '\0';
        }
        const char *ptr = *line;
        JConfParserState state = J_CONF_PARSER_STATE_NEW;
        const char *start = ptr;
        name = NULL;
        value = NULL;
        while (*ptr) {
            char c = *ptr;
            switch (state) {
            case J_CONF_PARSER_STATE_NEW:
                if (j_isscope(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE;
                } else if (isalpha(c)) {
                    start = ptr;
                    state = J_CONF_PARSER_STATE_DIRECTIVE_NAME;
                } else if (!j_isspace(c)) {
                    j_conf_parser_error(errstr,
                                        "[%s:%u] directive name must start with "
                                        "an alpha character", filepath,
                                        ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_DIRECTIVE_NAME:
                if (j_isspace(c)) {
                    name = j_strndup(start, ptr - start);
                    state = J_CONF_PARSER_STATE_DIRECTIVE_NAME_END;
                } else if (!j_isname(c)) {
                    j_conf_parser_error(errstr,
                                        "[%s:%u] directive name must only "
                                        "contain underline, alpha or digit "
                                        "character", filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_DIRECTIVE_NAME_END:
                if (!j_isspace(c)) {
                    start = ptr;
                    state = J_CONF_PARSER_STATE_DIRECTIVE_VALUE;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE:
                if (j_isspace(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_START;
                } else if (j_isend(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_END;
                } else if (isalpha(c)) {
                    start = ptr;
                    state = J_CONF_PARSER_STATE_SCOPE_START_NAME;
                } else {
                    j_conf_parser_error(errstr,
                                        "[%s:%u] invalid character in scope closure");
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_START:
                if (isalpha(c)) {
                    start = ptr;
                    state = J_CONF_PARSER_STATE_SCOPE_START_NAME;
                } else if (!j_isspace(c)) {
                    j_conf_parser_error(errstr, "[%s:%u] scope name "
                                        "must start with an alpha character",
                                        filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_START_NAME:
                if (j_isspace(c)) {
                    name = j_strndup(start, ptr - start);
                    state = J_CONF_PARSER_STATE_SCOPE_START_NAME_END;
                } else if (j_isclose(c)) {
                    name = j_strndup(start, ptr - start);
                    state = J_CONF_PARSER_STATE_SCOPE_START_END;
                } else if (!j_isname(c)) {
                    j_conf_parser_error(errstr,
                                        "[%s:%u] scope name must only "
                                        "contain underline, alpha or digit "
                                        "character", filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_START_NAME_END:
                if (j_isclose(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_START_END;
                } else if (!j_isspace(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_START_VALUE;
                    start = ptr;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_START_VALUE:
                if (j_isclose(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_START_END;
                    value = j_strndup(start, ptr - start);
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_START_END:
                if (!j_isspace(c)) {
                    j_conf_parser_error(errstr, "[%s:%u] "
                                        "redundant character after scope closure",
                                        filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_END:
                if (isalpha(c)) {
                    start = ptr;
                    state = J_CONF_PARSER_STATE_SCOPE_END_NAME;
                } else if (!j_isspace(c)) {
                    j_conf_parser_error(errstr, "[%s:%u] scope name "
                                        "must start with an alpha character",
                                        filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_END_NAME:
                if (j_isspace(c)) {
                    name = j_strndup(start, ptr - start);
                    state = J_CONF_PARSER_STATE_SCOPE_END_NAME_END;
                } else if (j_isclose(c)) {
                    name = j_strndup(start, ptr - start);
                    state = J_CONF_PARSER_STATE_SCOPE_END_END;
                } else if (!j_isname(c)) {
                    j_conf_parser_error(errstr,
                                        "[%s:%u] scope name must only "
                                        "contain underline, alpha or digit "
                                        "character", filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_END_NAME_END:
                if (j_isclose(c)) {
                    state = J_CONF_PARSER_STATE_SCOPE_END_END;
                } else if (!j_isspace(c)) {
                    j_conf_parser_error(errstr, "[%s:%u] scope end "
                                        "cannot contain argument",
                                        filepath, ln);
                    goto OUT;
                }
                break;
            case J_CONF_PARSER_STATE_SCOPE_END_END:
                if (!j_isspace(c)) {
                    j_conf_parser_error(errstr, "[%s:%u] "
                                        "redundant character after scope closure",
                                        filepath, ln);
                    goto OUT;
                }
                break;
            default:
                break;
            }
            ptr++;
        }

        JConfNode *node = NULL;
        switch (state) {
        case J_CONF_PARSER_STATE_DIRECTIVE_NAME:
            name = j_strndup(start, ptr - start);
        case J_CONF_PARSER_STATE_DIRECTIVE_NAME_END:
            if (j_strcmp0(INCLUDE_CONFIG, name) == 0) {
                j_conf_parser_error(errstr, "[%s:%u] no argument "
                                    "specified for %s", filepath, ln,
                                    INCLUDE_CONFIG);
                goto OUT;
            }
            node = j_conf_node_new_take(J_CONF_NODE_DIRECTIVE, name);
            j_conf_node_append_child((JConfNode *) j_stack_top(scopes),
                                     node);
            break;
        case J_CONF_PARSER_STATE_DIRECTIVE_VALUE:
            node = j_conf_node_new_take(J_CONF_NODE_DIRECTIVE, name);
            name = NULL;
            if (!j_conf_node_set_arguments_take(node,
                                                j_strndup(start,
                                                          ptr - start))) {
                j_conf_node_free(node);
                j_conf_parser_error(errstr, "[%s:%u] invalid "
                                    "directive argument", filepath, ln);
                goto OUT;
            }
            if (j_stack_length(scopes) == 1 &&
                j_strcmp0(INCLUDE_CONFIG,
                          j_conf_node_get_name(node)) == 0) {
                /* IncludeConf */
                JList *arg = j_conf_node_get_arguments(node);
                while (arg) {
                    JConfData *d = (JConfData *) j_list_data(arg);
                    const char *path = NULL;
                    if (j_conf_data_is_string(d)) {
                        path = j_conf_data_get_string(d);
                    } else {
                        path = j_conf_data_get_raw(d);
                    }
                    char **joined = j_conf_parser_get_path(parser, path);
                    if (joined == NULL) {
                        printf("[%s:%u] \'%s\' not found!\n",
                               filepath, ln, path);
                    } else {
                        char **file_path = joined;
                        while (*file_path) {
                            if (!j_conf_parser_parse
                                (parser, *file_path, errstr)) {
                                j_strfreev(joined);
                                j_conf_node_free(node);
                                goto OUT;
                            }
                            file_path++;
                        }
                        j_strfreev(joined);
                    }
                    arg = j_list_next(arg);
                }
                j_conf_node_free(node);
            } else {
                j_conf_node_append_child((JConfNode *) j_stack_top(scopes),
                                         node);
            }
            break;
        case J_CONF_PARSER_STATE_SCOPE:
        case J_CONF_PARSER_STATE_SCOPE_START:
        case J_CONF_PARSER_STATE_SCOPE_START_NAME:
        case J_CONF_PARSER_STATE_SCOPE_START_NAME_END:
        case J_CONF_PARSER_STATE_SCOPE_START_VALUE:
        case J_CONF_PARSER_STATE_SCOPE_END:
        case J_CONF_PARSER_STATE_SCOPE_END_NAME:
        case J_CONF_PARSER_STATE_SCOPE_END_NAME_END:
            j_conf_parser_error(errstr, "[%s:%u] unexpected EOL ",
                                filepath, ln);
            goto OUT;
            break;
        case J_CONF_PARSER_STATE_SCOPE_START_END:
            node = j_conf_node_new_take(J_CONF_NODE_SCOPE, name);
            name = NULL;
            if (value) {
                if (!j_conf_node_set_arguments_take(node, value)) {
                    value = NULL;
                    j_conf_node_free(node);
                    j_conf_parser_error(errstr, "[%s:%u] invalid "
                                        "scope argument", filepath, ln);
                    goto OUT;
                }
            }
            j_conf_node_append_child((JConfNode *) j_stack_top(scopes),
                                     node);
            j_stack_push(scopes, node);
            break;
        case J_CONF_PARSER_STATE_SCOPE_END_END:
            node = (JConfNode *) j_stack_pop(scopes);
            if (j_strcmp0(name, j_conf_node_get_name(node))) {
                j_conf_parser_error(errstr, "[%s:%u] scope name "
                                    "doesn\'t match", filepath, ln);
                goto OUT;
            }
            j_free(name);
            name = NULL;
            break;
        default:
            break;

        }
        line++;
    }
    ret = 1;
  OUT:
    j_free(name);
    j_free(value);
    j_stack_free(scopes, NULL);
    j_strfreev(lines);
    return ret;
}
