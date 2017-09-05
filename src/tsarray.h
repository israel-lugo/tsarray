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


/* Private metadata for the tsarray. For internal use only. */
struct _tsarray_metadata {
    size_t capacity;
};


/* Abstract version; only for internal use (must match the subclassed
 * versions in TSARRAY_TYPEDEF) */
struct _tsarray_abs {
    size_t len;
    void *items;    /* placeholder */
    struct _tsarray_metadata _priv;
};


/*
 * TODO: We can actually hide the _priv field from the user, with some pointer
 * magic. Create an internal-only struct _tsarray_desc with all the internal
 * fields we want, and the last field is the publicly accessible struct
 * _tsarray_abs (which only has user-visible fields). We'll need a constructor
 * to create tsarrays. The constructor will internally create a _tsarray_desc,
 * but will only return a pointer to the public struct _tsarray_abs within it.
 * All public functions receive that pointer, it's the only thing the user
 * ever knows. Internally, whenever we want to access the containing struct
 * _tsarray_desc of a given struct _tsarray_abs, we use offsetof() pointer
 * arithmetic to calculate the container address.
 */


struct _tsarray_abs *tsarray_new(void) __ATTR_MALLOC;

struct _tsarray_abs *tsarray_from_array(const void *src, size_t src_len,
        size_t obj_size) __ATTR_MALLOC;

int tsarray_append(struct _tsarray_abs *p_tsarray, const void *object,
        size_t obj_size) __NON_NULL;

int tsarray_extend(struct _tsarray_abs *p_tsarray_dest,
        struct _tsarray_abs *p_tsarray_src, size_t obj_size) __NON_NULL;

int tsarray_remove(struct _tsarray_abs *p_tsarray, int index,
        size_t obj_size) __NON_NULL;

void tsarray_free(struct _tsarray_abs *p_tsarray) __NON_NULL;


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
    typedef struct { \
        size_t len; \
        objtype *items; \
        struct _tsarray_metadata _priv; \
    } arraytype; \
    static inline arraytype *arraytype##_new(void) { \
        return (arraytype *)tsarray_new(); \
    } \
    static inline arraytype *arraytype##_from_array(const void *src, \
            size_t src_len) { \
        return (arraytype *)tsarray_from_array(src, src_len, sizeof(objtype)); \
    } \
    static inline int arraytype##_append(arraytype *array, objtype *object) { \
        return tsarray_append((struct _tsarray_abs *)array, object, \
                sizeof(objtype)); \
    } \
    static inline int arraytype##_extend(arraytype *dest, arraytype *src) { \
        return tsarray_extend((struct _tsarray_abs *)dest, \
                (struct _tsarray_abs *)src, sizeof(objtype)); \
    } \
    static inline int arraytype##_remove(arraytype *array, size_t index) { \
        return tsarray_remove((struct _tsarray_abs *)array, index, \
                sizeof(objtype)); \
    } \
    static inline void arraytype##_free(arraytype *array) { \
        tsarray_free((struct _tsarray_abs *)array); \
    }


#endif      /* not _TSARRAY_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=78 : */
