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
 * compiler.h - compiler-dependent definitions
 */


#ifndef _COMPILER_H
#define _COMPILER_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif



/* Branch prediction stuff */
#ifdef __GNUC__
#  define likely(x)     __builtin_expect(!!(x), 1)
#  define unlikely(x)   __builtin_expect(!!(x), 0)
#else   /* not gcc */
#  define likely(x)     (x)
#  define unlikely(x)   (x)
#  define __attribute__(x)
#endif  /* not gcc */



/*
 * Functions marked with __ATTR_CONST do not examine any values except
 * their arguments, and have no effects except the return value. This
 * is a stricter class than __ATTR_PURE (see below), since the function
 * is not allowed to read global memory. Such a function can be subject
 * to common subexpression elimination and loop optimization in GCC.
 *
 * Functions marked with __ATTR_MALLOC behave like malloc: any non-NULL
 * pointer the function returns cannot alias any other pointer valid
 * when the function returns. This helps GCC optimize better in some
 * cases.
 *
 * Functions marked with __ATTR_PURE have no effects except the
 * return value and their return value depends only on the parameters
 * and/or global variables. Such a function can be subject to common
 * subexpression elimination and loop optimization in GCC.
 *
 * Functions marked with __NON_NULL cannot receive a NULL pointer in any
 * of their pointer arguments. With this information, the compiler can
 * check for correctness at compile-time (issuing a warning if -Wnonnull
 * is enabled) and possibly do certain optimizations by disregarding the
 * NULL pointer case.
 */
#if defined(__GNUC__) && __GNUC__ >= 3
#  define __ATTR_CONST      __attribute__((__const__))
#  define __ATTR_MALLOC     __attribute__((__malloc__))
#  define __ATTR_PURE       __attribute__((__pure__))
#  define __NON_NULL        __attribute__((__nonnull__))
#  define __MAYBE_UNUSED    __attribute__((__unused__))
#else
#  define __ATTR_CONST
#  define __ATTR_MALLOC
#  define __ATTR_PURE
#  define __NON_NULL
#  define __MAYBE_UNUSED
#endif


/* Return the offset of MEMBER inside a structure of type TYPE */
#if defined(__GNUC__) && __GNUC__ >= 4
#  define _offsetof(type, member) __builtin_offsetof(type, member)
#else
#  define _offsetof(type, member) ((size_t) &(((type *)0)->member))
#endif



/*
 * Given a pointer (P_EL) to a field named MEMBER in a container of type
 * CONT_TYPE, return the container's address.
 */
#define _CONTAINER_ADDR(p_el, cont_type, member) \
    ((cont_type *)((char *)(p_el) - _offsetof(cont_type, member)))



#ifdef __GNUC__
#  define __ATTR_PACKED __attribute__((packed))
#else
#  define __ATTR_PACKED
#endif

#endif  /* _COMPILER_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
