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
 * tsarray.h - dynamic array module header
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


/* abstract versions; only for internal use (must match the subclassed
 * versions in TSARRAY_TYPEDEF) */
struct _tsarray_abs {
    int len;
    int used_count;
    int min_len;
    struct _item_abs *items;
};


int tsarray_add(struct _tsarray_abs *p_tsarray, const void *object,
        size_t obj_size, size_t item_size) __attribute__((nonnull (1)));

int tsarray_remove(struct _tsarray_abs *p_tsarray, int index,
        size_t item_size) __NON_NULL;

int tsarray_compact(struct _tsarray_abs *p_tsarray, int force,
        size_t obj_size, size_t item_size) __NON_NULL;

int tsarray_truncate(struct _tsarray_abs *p_tsarray, int len,
        size_t item_size) __NON_NULL;

int tsarray_setminlen(struct _tsarray_abs *p_tsarray, int min_len,
        size_t item_size) __NON_NULL;


/* Initializer for an empty tsarray. May be used directly as initializer on
 * a declaration, or as rvalue on an assignment expression (for an already
 * declared identifier). In the latter case, this must be must be transformed
 * into a compound literal (by prepending the type name in parenthesis), e.g.:
 *      a1 = (intarray)TSARRAY_INITIALIZER;
 */
#define TSARRAY_INITIALIZER { 0, 0, 0, NULL }


/*
 * Declare a new type-specific tsarray type.
 *
 * Defines (typedefs) arraytype to the new tsarray type, which will store
 * objects of type objtype. Defines type-specific functions to manipulate the
 * new array type using the prefix arraytype_*, e.g. intarray_add(), etc.
 *
 * Example (define intarray as an array of int):
 *      TSARRAY_TYPEDEF(intarray, int);
 */
#define TSARRAY_TYPEDEF(arraytype, objtype) \
    struct arraytype##_item { \
        int used; \
        objtype object; \
    }; \
    typedef struct { \
        int len; \
        int used_count; \
        int min_len; \
        struct arraytype##_item *items; \
    } arraytype; \
    static inline int arraytype##_add(arraytype *array, objtype *object) { \
        return tsarray_add((struct _tsarray_abs *)array, object, \
                            sizeof(objtype), sizeof(struct arraytype##_item)); \
    } \
    static inline int arraytype##_remove(arraytype *array, int index) { \
        return tsarray_remove((struct _tsarray_abs *)array, index, \
                               sizeof(struct arraytype##_item)); \
    } \
    static inline objtype *arraytype##_get_nth(arraytype *array, int index) { \
        struct arraytype##_item *item = &array->items[index]; \
        return likely(item->used) ? &item->object : NULL; \
    } \
    static inline int arraytype##_compact(arraytype *array, int force) { \
        return tsarray_compact((struct _tsarray_abs *)array, force, \
                                sizeof(objtype), sizeof(struct arraytype##_item)); \
    } \
    static inline int arraytype##_truncate(arraytype *array, int len) { \
        return tsarray_truncate((struct _tsarray_abs *)array, len, \
                                 sizeof(struct arraytype##_item)); \
    } \
    static inline int arraytype##_setminlen(arraytype *array, int len) { \
        return tsarray_setminlen((struct _tsarray_abs *)array, len, \
                                  sizeof(struct arraytype##_item)); \
    }



#endif      /* not _TSARRAY_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=78 : */
