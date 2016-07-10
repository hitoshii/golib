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

package main

import (
	"../src/log"
)

func main() {
	cfg := log.LogConfig{
		Namespace:     "",
		ShowNamespace: false,
		Loggers: []log.LoggerConfig{
			{"DEBUG", "STDOUT"},
			{"ERROR", "STDERR"},
			{"WARN", "./warn.output"},
		},
	}
	log.Init(&cfg)
	log.SetDefault(cfg.Namespace)
	log.D("hello %d", 1)
	log.E("world %s", "nice")
	log.W("oh no")

	cfg = log.LogConfig{
		Namespace:     "logger",
		ShowNamespace: true,
		Loggers: []log.LoggerConfig{
			{"DEBUG", "STDOUT"},
			{"ERROR", "STDERR"},
			{"WARN", "./warn.output"},
		},
	}
	log.Init(&cfg)
	log.SetDefault(cfg.Namespace)
	log.D("hello again %f", 2.3)
	log.E("world again %c", 'c')
	log.W("oh no")
}
