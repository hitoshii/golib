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
#ifndef __J_CONF_PARSER_H__
#define __J_CONF_PARSER_H__


#include "node.h"

typedef struct _JConfParser JConfParser;


JConfParser *j_conf_parser_new();

/*
 * Frees JConfParser
 */
void j_conf_parser_free(JConfParser * parser);

/*
 * str must contain a '=',
 * it should be something like "DefaultLog=/var/log/log"
 *          Then {DefaultLog} in conf file will be replaced as /var/log/log 
 * If the format of env is invalid, then nothing will be added
 */
void j_conf_parser_add_variable(JConfParser * parser, const char *str);

/*
 * Adds a enveriment path
 */
void j_conf_parser_add_env(JConfParser * parser, const char *str);


/*
 * Parses a conf file,
 * If error occurs, errstr will be set the error string (optional), and returns 0
 * Returns 1 on success
 */
int j_conf_parser_parse(JConfParser * parser, const char *filepath,
                        char **errstr);


#endif
