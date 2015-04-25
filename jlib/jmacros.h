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

#ifndef __JLIB_MACROS_H__
#define __JLIB_MACROS_H__

#include <stddef.h>

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define J_GNUC_EXTENSION __extension__
#else
#define J_GNUC_EXTENSION
#endif

/* Provide macros to feature the GCC function attribute.
 */
#if    __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define J_GNUC_PURE __attribute__((__pure__))
#define J_GNUC_MALLOC __attribute__((__malloc__))
#else
#define J_GNUC_PURE
#define J_GNUC_MALLOC
#endif

#define JPOINTER_TO_JUINT(p) ((juint) (p))
#define JUINT_TO_POINTER(u) ((jpointer)(juint) u)

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _J_BOOLEAN_EXPR(expr)                   \
 J_GNUC_EXTENSION ({                            \
   int _j_boolean_var_;                         \
   if (expr)                                    \
      _j_boolean_var_ = 1;                      \
   else                                         \
      _j_boolean_var_ = 0;                      \
   _j_boolean_var_;                             \
})
#define J_LIKELY(expr) (__builtin_expect ( _J_BOOLEAN_EXPR(expr), 1))
#define J_UNLIKELY(expr) (__builtin_expect ( _J_BOOLEAN_EXPR(expr), 0))
#else
#define J_LIKELY(expr) (expr)
#define J_UNLIKELY(expr) (expr)
#endif


#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#undef ABS
#define ABS(a)  (((a) < 0) ? -(a) : (a))

/*
 * Make sure x is between low and high.
 * If low is greater than high, result is undefined
 */
#undef CLAMP
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define J_GNUC_PRINTF( format_idx, arg_idx )    \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define J_GNUC_SCANF( format_idx, arg_idx )     \
  __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define J_GNUC_FORMAT( arg_idx )                \
  __attribute__((__format_arg__ (arg_idx)))
#define J_GNUC_NORETURN                         \
  __attribute__((__noreturn__))
#define J_GNUC_CONST                            \
  __attribute__((__const__))
#define J_GNUC_UNUSED                           \
  __attribute__((__unused__))
#define J_GNUC_NO_INSTRUMENT			\
  __attribute__((__no_instrument_function__))
#else                           /* !__GNUC__ */
#define J_GNUC_PRINTF( format_idx, arg_idx )
#define J_GNUC_SCANF( format_idx, arg_idx )
#define J_GNUC_FORMAT( arg_idx )
#define J_GNUC_NORETURN
#define J_GNUC_CONST
#define J_GNUC_UNUSED
#define J_GNUC_NO_INSTRUMENT
#endif                          /* !__GNUC__ */

#ifndef __GI_SCANNER__          /* The static assert macro really confuses the introspection parser */
#define J_PASTE_ARGS(identifier1,identifier2) identifier1 ## identifier2
#define J_PASTE(identifier1,identifier2)      J_PASTE_ARGS (identifier1, identifier2)
#ifdef __COUNTER__
#define J_STATIC_ASSERT(expr) typedef char J_PASTE (_GStaticAssertCompileTimeAssertion_, __COUNTER__)[(expr) ? 1 : -1] J_GNUC_UNUSED
#else
#define J_STATIC_ASSERT(expr) typedef char J_PASTE (_GStaticAssertCompileTimeAssertion_, __LINE__)[(expr) ? 1 : -1] J_GNUC_UNUSED
#endif
#define J_STATIC_ASSERT_EXPR(expr) ((void) sizeof (char[(expr) ? 1 : -1]))
#endif



#endif
