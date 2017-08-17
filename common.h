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
#else   /* not gcc */
#  define min(x, y) ((x) < (y) ? (x) : (y))
#  define max(x, y) ((x) > (y) ? (x) : (y))
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


static inline int can_sadd(const int x, const int y) __ATTR_CONST;


/*
 * Check whether two signed integers can be added without overflowing.
 */
static inline int can_sadd(const int x, const int y)
{
    return likely((x <= 0 || y <= INT_MAX - x)
                  && (x >= 0 || y >= INT_MIN - x));
}

/*
 * Check whether two size_t values can be added without overflowing.
 */
static inline int can_size_add(const size_t x, const size_t y)
{   /* size_t is unsigned by definition */
    return x <= (SIZE_MAX - y);
}


#endif  /* _COMMON_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
