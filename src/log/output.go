/*
 * Copyright (C) 2016 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

package log

import (
	"strings"
)

var (
	gDefaultNamespace = ""
)

func output(namespace, level, fmt string, v ...interface{}) {
	if loggers := gLoggers[namespace]; loggers != nil {
		if logger := loggers[level]; logger != nil {
			logger.Printf(fmt, v...)
		}
	}
}

func MESSAGE(namespace, level, fmt string, v ...interface{}) {
	output(namespace, strings.ToUpper(level), fmt, v...)
}

/* 设置默认的命名空间 */
func SetDefault(namespace string) {
	gDefaultNamespace = namespace
}

func DEBUG(fmt string, v ...interface{}) {
	output(gDefaultNamespace, "DEBUG", fmt, v...)
}

func INFO(fmt string, v ...interface{}) {
	output(gDefaultNamespace, "INFO", fmt, v...)
}

func WARNING(fmt string, v ...interface{}) {
	output(gDefaultNamespace, "WARNING", fmt, v...)
}

func ERROR(fmt string, v ...interface{}) {
	output(gDefaultNamespace, "ERROR", fmt, v...)
}
