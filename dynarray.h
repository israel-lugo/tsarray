/*
 * libncraft - network crafting library
 * Copyright 2005, 2006, 2007, 2008, 2009 Israel G. Lugo
 *
 * This file is part of libncraft.
 *
 * libncraft is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libncraft is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libncraft.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For suggestions, feedback or bug reports: israel.lugo@lugosys.com
 */



/*
 * dynarray.h - dynamic array module header
 */

/* $Id: dynarray.h 267 2012-06-13 19:47:59Z capi $ */

#ifndef _DYNARRAY_H
#define _DYNARRAY_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif


/* get NULL and size_t */
#include <stddef.h>

/* get memory allocation */
#include <stdlib.h>


#include "common.h"



/* abstract versions; only for internal use (must match the subclassed
 * versions in DYNARRAY_TYPE_DECLARE) */
struct _dynarray_abs {
    int len;
    int used_count;
    struct _item_abs *items;
};


int dynarray_add(struct _dynarray_abs *p_dynarray, const void *object,
        size_t obj_size, size_t item_size) __NON_NULL;

int dynarray_remove(struct _dynarray_abs *p_dynarray, int index,
        size_t item_size) __NON_NULL;

int dynarray_compact(struct _dynarray_abs *p_dynarray, int force,
        size_t obj_size, size_t item_size) __NON_NULL;


/* Initializer for an empty dynarray. When assigning to an already declared
 * array, this must be transformed into a compound literal (by prepending
 * the type name in parenthesis), e.g.: (intarray)DYNARRAY_EMPTY
 */
#define DYNARRAY_EMPTY { 0, 0, NULL }


#define DYNARRAY_TYPE_DECLARE(name, type) \
    struct name##_item { \
        int used; \
        type object; \
    }; \
    typedef struct { \
        int len; \
        int used_count; \
        struct name##_item *items; \
    } name; \
    static inline int name##_add(name *array, type *object) { \
        return dynarray_add((struct _dynarray_abs *)array, object, \
                            sizeof(type), sizeof(struct name##_item)); \
    } \
    static inline int name##_remove(name *array, int index) { \
        return dynarray_remove((struct _dynarray_abs *)array, index, \
                               sizeof(struct name##_item)); \
    } \
    static inline type *name##_get_nth(name *array, int index) { \
        struct name##_item *item = &array->items[index]; \
        return likely(item->used) ? &item->object : NULL; \
    } \
    static inline int name##_compact(name *array, int force) { \
        return dynarray_compact((struct _dynarray_abs *)array, force, \
                                sizeof(type), sizeof(struct name##_item)); \
    }



#endif      /* not _DYNARRAY_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
