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
 * tsarray.h - generic type-safe dynamic array module header
 */


#ifndef _TSARRAY_H
#define _TSARRAY_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif


/* get NULL and size_t */
#include <stddef.h>

/* get memory allocation */
#include <stdlib.h>


#include "common.h"


/*
 * Error values returned by API functions. Always negative in case of error.
 */
enum tsarray_errno {
    TSARRAY_EOK = 0,        /* Success */
    TSARRAY_EINVAL = -1,    /* Invalid argument */
    TSARRAY_ENOENT = -2,    /* No such entry */
    TSARRAY_ENOMEM = -3,    /* Out of memory */
    TSARRAY_EOVERFLOW = -4, /* Operation would overflow */
};


/* Abstract version; only for internal use (must match the subclassed
 * versions in TSARRAY_TYPEDEF) */
struct _tsarray_pub {
    char *items;    /* char may alias any other type (C99 6.5.2.3) */
};


struct _tsarray_pub *tsarray_new(size_t obj_size) __ATTR_MALLOC;

struct _tsarray_pub *tsarray_from_array(const void *src, size_t src_len,
        size_t obj_size) __ATTR_MALLOC;

struct _tsarray_pub *tsarray_copy(const struct _tsarray_pub *tsarray_src)
    __NON_NULL __ATTR_MALLOC;

size_t tsarray_len(const struct _tsarray_pub *tsarray) __ATTR_CONST __NON_NULL;

int tsarray_append(struct _tsarray_pub *tsarray, const void *object) __NON_NULL;

int tsarray_extend(struct _tsarray_pub *tsarray_dest,
        struct _tsarray_pub *tsarray_src) __NON_NULL;

struct _tsarray_pub *tsarray_slice(const struct _tsarray_pub *p_tsarray,
        size_t start, size_t stop, size_t step) __NON_NULL __ATTR_MALLOC;

int tsarray_remove(struct _tsarray_pub *p_tsarray, int index) __NON_NULL;

void tsarray_free(struct _tsarray_pub *p_tsarray) __NON_NULL;


/*
 * Declare a new type-specific tsarray type.
 *
 * Defines (typedefs) arraytype to the new tsarray type, which will store
 * objects of type objtype. Defines type-specific functions to manipulate the
 * new array type using the prefix arraytype_*, e.g. intarray_append(), etc.
 *
 * Example (define intarray as an array of int):
 *      TSARRAY_TYPEDEF(intarray, int);
 */
#define TSARRAY_TYPEDEF(arraytype, objtype) \
    typedef struct { objtype *items; } arraytype; \
    static inline arraytype *arraytype##_new(void) { \
        return (arraytype *)tsarray_new(sizeof(objtype)); \
    } \
    static inline arraytype *arraytype##_from_array(const void *src, \
            size_t src_len) { \
        return (arraytype *)tsarray_from_array(src, src_len, sizeof(objtype)); \
    } \
    static inline arraytype *arraytype##_copy(const arraytype *array) { \
        return (arraytype *)tsarray_copy((const struct _tsarray_pub *)array); \
    } \
    static inline size_t arraytype##_len(const arraytype *array) { \
        return tsarray_len((const struct _tsarray_pub *)array); \
    } \
    static inline int arraytype##_append(arraytype *array, objtype *object) { \
        return tsarray_append((struct _tsarray_pub *)array, object); \
    } \
    static inline int arraytype##_extend(arraytype *dest, arraytype *src) { \
        return tsarray_extend((struct _tsarray_pub *)dest, \
                (struct _tsarray_pub *)src); \
    } \
    static inline int arraytype##_remove(arraytype *array, size_t index) { \
        return tsarray_remove((struct _tsarray_pub *)array, index); \
    } \
    static inline arraytype *arraytype##_slice(const arraytype *array, \
            size_t start, size_t stop, size_t step) { \
        return (arraytype *)tsarray_slice((const struct _tsarray_pub *)array, \
                start, stop, step); \
    } \
    static inline void arraytype##_free(arraytype *array) { \
        tsarray_free((struct _tsarray_pub *)array); \
    }


#endif      /* not _TSARRAY_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=78 : */
