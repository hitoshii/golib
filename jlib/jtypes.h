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
#ifndef __J_LIB_TYPES_H__
#define __J_LIB_TYPES_H__

#include <stdint.h>
#include "jmacros.h"

// typedef char char;
// typedef short short;
// typedef long long;
// typedef int int;
typedef int boolean;

// typedef unsigned char unsigned char;
// typedef unsigned short unsigned short;
// typedef unsigned long unsigned long;
// typedef unsigned int unsigned int;

// typedef float float;
// typedef double double;

// typedef int8_t int8_t;
// typedef int16_t int16_t;
// typedef int32_t int32_t;
// typedef int64_t int64_t;

// typedef uint8_t uint8_t;
// typedef uint16_t uint16_t;
// typedef uint32_t uint32_t;
// typedef uint64_t uint64_t;

// typedef signed long signed long;
// typedef unsigned long unsigned long;

// typedef void *void *;
// typedef const void *const void *;

#define J_MININT8   ((int8_t) 0x80)
#define J_MAXINT8   ((int8_t) 0x7F)
#define J_MAXUINT8  ((uint8_t) 0xFF)

#define J_MININT16  ((int16_t) 0x8000)
#define J_MAXINT16  ((int16_t) 0x7FFF)
#define J_MAXUINT16 ((uint16_t) 0xFFFF)

#define J_MININT32  ((int32_t) 0x80000000)
#define J_MAXINT32  ((int32_t) 0x7FFFFFFF)
#define J_MAXUINT32 ((uint32_t) 0xFFFFFFFF)

#define J_MININT64  ((int64_t) 0x8000000000000000)
#define J_MAXINT64  ((int64_t) 0x7FFFFFFFFFFFFFFF)
#define J_MAXUINT64 ((uint64_t) 0xFFFFFFFFFFFFFFFFU)

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

#define J_BIG_ENDIAN    4321
#define J_LITTLE_ENDIAN 1234

/*
 * Functions
 */
typedef void (*JDestroyNotify) (void * data);
typedef int(*JCompareFunc) (const void * data, const void * user_data);
typedef int(*JCompareDataFunc) (const void * a, const void * b,
                                const void * user_data);
typedef void (*JFunc) (void * data, void * user_data);

#endif
