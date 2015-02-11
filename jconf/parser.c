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
#include <jlib/jlib.h>


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
};


JConfParser *j_conf_parser_new()
{
    JConfParser *parser = (JConfParser *) j_malloc(sizeof(JConfParser));
    parser->vars = NULL;
    parser->envs = NULL;
    parser->root = j_conf_node_new(J_CONF_NODE_SCOPE, NULL);
    return parser;
}

/*
 * Frees JConfParser
 */
void j_conf_parser_free(JConfParser * parser)
{
    j_list_free_full(parser->vars, (JListDestroy) j_conf_var_free);
    j_list_free_full(parser->envs, (JListDestroy) free);
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


/*
 * Parses a conf file,
 * If error occurs, errstr will be set the error string (optional), and returns 0
 * Returns 1 on success
 */
int j_conf_parser_parse(JConfParser * parser, const char *filepath,
                        char **errstr)
{
}
