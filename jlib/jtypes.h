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
#ifndef __J_LIB_TYPES_H__
#define __J_LIB_TYPES_H__

#include <stdint.h>
#include "jmacros.h"

typedef char jchar;
typedef short jshort;
typedef long jlong;
typedef int jint;
typedef jint jboolean;

typedef unsigned char juchar;
typedef unsigned short jushort;
typedef unsigned long julong;
typedef unsigned int juint;

typedef float jfloat;
typedef double jdouble;

typedef int8_t jint8;
typedef int16_t jint16;
typedef int32_t jint32;
typedef int64_t jint64;

typedef uint8_t juint8;
typedef uint16_t juint16;
typedef uint32_t juint32;
typedef uint64_t juint64;

typedef signed long jssize;
typedef unsigned long jsize;

typedef void *jpointer;
typedef const void *jconstpointer;

#define J_MININT8   ((jint8) 0x80)
#define J_MAXINT8   ((jint8) 0x7F)
#define J_MAXUINT8  ((juint8) 0xFF)

#define J_MININT16  ((jint16) 0x8000)
#define J_MAXINT16  ((jint16) 0x7FFF)
#define J_MAXUINT16 ((juint16) 0xFFFF)

#define J_MININT32  ((jint32) 0x80000000)
#define J_MAXINT32  ((jint32) 0x7FFFFFFF)
#define J_MAXUINT32 ((juint32) 0xFFFFFFFF)

#define J_MININT64  ((jint64) 0x8000000000000000)
#define J_MAXINT64  ((jint64) 0x7FFFFFFFFFFFFFFF)
#define J_MAXUINT64 ((juint64) 0xFFFFFFFFFFFFFFFFU)

/*
 * Constant
 */
#define J_E     2.7182818284590452353602874713526624977572470937000
#define J_LN2   0.69314718055994530941723212145817656807550013436026
#define J_LN10  2.3025850929940456840179914546843642076011014886288
#define J_PI    3.1415926535897932384626433832795028841971693993751
#define J_PI_2  1.5707963267948966192313216916397514420985846996876
#define J_PI_4  0.78539816339744830961566084581987572104929234984378
#define J_SQRT2 1.4142135623730950488016887242096980785696718753769

/*
 * Functions
 */
typedef void (*JDestroyNotify) (jpointer data);
typedef jint(*JCompareFunc) (jconstpointer data, jconstpointer user_data);
typedef jint(*JCompareDataFunc) (jconstpointer a, jconstpointer b,
                                 jconstpointer user_data);
typedef void (*JFunc) (jpointer data, jpointer user_data);

#endif
