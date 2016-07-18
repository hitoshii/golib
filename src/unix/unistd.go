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

package unix

//#include <unistd.h>
import "C"

/*
 * man daemon
 *
 * The daemon() function is for programs wishing to detach themselves from
 * the controlling terminal and run in the background as system daemons.
 *
 * If nochdir is zero, daemon()  changes  the  process's  current  working
 * directory  to  the root directory ("/"); otherwise, the current working
 * directory is left unchanged.
 *
 * If noclose is zero, daemon() redirects standard input, standard  output
 * and  standard  error  to  /dev/null;  otherwise, no changes are made to
 * these file descriptors.
 *
 * this function should be called before any goroutine created,
 * otherwise its behavior is undefined
 */
func Daemon(nochdir, noclose int) {
	C.daemon(C.int(nochdir), C.int(noclose))
}
