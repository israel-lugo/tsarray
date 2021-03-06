/*
 * tsarray - type-safe dynamic array library
 * Copyright 2012, 2015, 2016, 2017 Israel G. Lugo
 *
 * This file is part of tsarray.
 *
 * tsarray is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tsarray is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tsarray.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For suggestions, feedback or bug reports: israel.lugo@lugosys.com
 */



/*
 * common.h - common definitions header
 */


#ifndef _COMMON_H
#define _COMMON_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif


#include <stdlib.h>

/* get INT_MIN and INT_MAX */
#include <limits.h>

/* get SIZE_MAX */
#include <stdint.h>

#if !defined(SIZE_MAX)
#  define SIZE_MAX (~((size_t)0))
#endif


#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#else
#  ifndef HAVE__BOOL
#    ifdef __cplusplus
typedef bool _Bool;
#    else
#      define _Bool signed char
#    endif
#  endif
#  define bool _Bool
#  define false 0
#  define true 1
#  define __bool_true_false_are_defined 1
#endif


#if !defined(DEBUG) || !DEBUG
#  define NDEBUG 1
#endif
#include <assert.h>


#include "compiler.h"


/* Some utility macros */
#ifdef __GNUC__
#  define min(x, y) ({  \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    _x < _y ? _x : _y;  \
  })
#  define max(x, y) ({  \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    _x > _y ? _x : _y;  \
  })
#  define _abs(x) ({    \
    __typeof__(x) _x = (x); \
    _x < 0 ? -_x : _x;  \
  })
#else   /* not gcc */
#  define min(x, y) ((x) < (y) ? (x) : (y))
#  define max(x, y) ((x) > (y) ? (x) : (y))
#  define _abs(x) ((x) < 0 ? -(x) : (x))
#endif  /* not gcc */


/* Copyright message to be used in printed messages */
#define COPYRIGHT "Copyright (C) 2012, 2015, 2016, 2017 Israel G. Lugo"



#if defined(__GNUC__)
/* Define a public alias for a private function. Must be called in the same
 * translation unit where the private function is defined. The private
 * function must not be static. Only works with GCC.
 *      Example: EXPORT_FUNCTION_AS(msg_out, pc_msg_out)
 */
#  define EXPORT_FUNCTION_AS(private_func, public_func) \
   extern __typeof__(private_func) public_func \
       __attribute__((alias(#private_func), visibility("default")));
#endif


static inline int can_int_add(const int x, const int y) __ATTR_CONST;
static inline int can_long_add(const long x, const long y) __ATTR_CONST;
static inline int can_ulong_add(const unsigned long x,
        const unsigned long y) __ATTR_CONST;
static inline unsigned long ulong_add(const unsigned long x,
        const unsigned long y) __ATTR_CONST;
static inline int can_add_ulong_within(const unsigned long x,
        const unsigned long y, const unsigned long cap) __ATTR_CONST;
static inline unsigned long ulong_add_capped(const unsigned long x,
        const unsigned long y, const unsigned long cap) __ATTR_CONST;
static inline int can_add_within_long(const unsigned long x,
        const unsigned long y) __ATTR_CONST;
static inline unsigned long ulong_add_capped_long(const unsigned long x,
        const unsigned long y) __ATTR_CONST;
static inline int can_size_add(const size_t x, const size_t y) __ATTR_CONST;
static inline int can_size_mult(const size_t x, const size_t y) __ATTR_CONST;
static inline int is_valid_index(const unsigned long x,
        const size_t obj_size) __ATTR_CONST;


/*
 * Check whether two signed integers can be added without overflowing.
 */
static inline int can_int_add(const int x, const int y)
{
    return likely((y <= 0 || x <= INT_MAX - y)
                  && (y >= 0 || x >= INT_MIN - y));
}


/*
 * Check whether two signed long integers can be added without overflowing.
 */
static inline int can_long_add(const long x, const long y)
{
    return likely((y <= 0 || x <= LONG_MAX - y)
                  && (y >= 0 || x >= LONG_MIN - y));
}


/*
 * Check whether two unsigned long can be added without overflowing.
 */
static inline int can_ulong_add(const unsigned long x, const unsigned long y)
{
    return x <= (ULONG_MAX - y);
}


/*
 * Add two unsigned long, without overflowing.
 *
 * If the result of adding x and y would overflow, return ULONG_MAX.
 */
static inline unsigned long ulong_add(const unsigned long x,
        const unsigned long y)
{
    return likely(can_ulong_add(x, y)) ? x + y : ULONG_MAX;
}


/*
 * Check whether two unsigned long can be added within a cap.
 */
static inline int can_add_ulong_within(const unsigned long x,
        const unsigned long y, const unsigned long cap)
{
    return y <= cap && x <= cap && x <= (cap - y);
}


/*
 * Add two unsigned long, without going over a cap.
 *
 * If the result of adding x and y would be larger than cap, return cap.
 */
static inline unsigned long ulong_add_capped(const unsigned long x,
        const unsigned long y, const unsigned long cap)
{
    return can_add_ulong_within(x, y, cap) ? x+y : cap;
}


/*
 * Check whether two unsigned long can be added without overflowing as longs.
 */
static inline int can_add_within_long(const unsigned long x, const unsigned long y)
{
    return can_add_ulong_within(x, y, (unsigned long)LONG_MAX);
}


/*
 * Add two unsigned long, capped at maximum long.
 */
static inline unsigned long ulong_add_capped_long(const unsigned long x,
        const unsigned long y)
{
    return ulong_add_capped(x, y, (unsigned long)LONG_MAX);
}


/*
 * Check whether two signed long integers can be multiplied without overflowing.
 */
static inline int can_long_mult(const long x, const long y)
{
    /* avoid division by zero */
    if (y == 0)
        return 1;

    /* don't use the general solution for y == -1; two's-complement
     * architectures can't represent LONG_MIN/(-1) */
    if (y == -1)
        return x >= -LONG_MAX;

    const long max_over_y = LONG_MAX/y;
    const long min_over_y = LONG_MIN/y;

    return y > 0 ? (x <= max_over_y && x >= min_over_y)
                    : (x >= max_over_y && x <= min_over_y);
}


/*
 * Check whether an unsigned long's value would fit in a signed long.
 *
 * Useful to make sure conversion is possible, without risking overflow.
 */
static inline int ulong_fits_in_long(const unsigned long x)
{
    return x <= (unsigned long)LONG_MAX;
}


/*
 * Check whether two size_t values can be added without overflowing.
 */
static inline int can_size_add(const size_t x, const size_t y)
{   /* size_t is unsigned by definition */
    return x <= (SIZE_MAX - y);
}


/*
 * Check whether two size_t values can be multiplied without overflowing.
 */
static inline int can_size_mult(const size_t x, const size_t y)
{
    /* avoid division by zero, and trivial case where y==1 */
    return (y <= 1) || (x <= SIZE_MAX/y);
}


/*
 * Convert a size_t to a signed long integer, capping at LONG_MAX.
 *
 * This function must be used, instead of a direct cast, to protect from
 * cases where the (unsigned) value of x is too large to be represented in
 * a (signed) long integer. Per C99, that would be undefined behavior. In
 * such a case, this function returns LONG_MAX.
 */
static inline long size_to_long(const size_t x)
{
    return (x > (unsigned long)LONG_MAX) ? LONG_MAX : (long)x;
}


/*
 * Check if x is a valid index for an array of specified object size.
 *
 * Returns true if and only if x fits in a signed long, and represents an
 * addressable byte position (x * obj_size <= SIZE_MAX). obj_size MUST be
 * non-zero.
 */
static inline int is_valid_index(const unsigned long x, const size_t obj_size)
{
    return (x <= (unsigned long)LONG_MAX
            && x <= SIZE_MAX                /* remaining math makes sense */
            && can_size_mult(x, obj_size)           /* can address last item */
            && SIZE_MAX - x*obj_size >= obj_size-1);    /* last item fits */
}


#endif  /* _COMMON_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
