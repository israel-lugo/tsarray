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
 * dynarray.c - dynamic array module
 */

/* $Id$ */


#if HAVE_CONFIG_H
#  include <config.h>
#endif


/* get memcpy */
#include <string.h>

#include "dynarray.h"
#include "common.h"



/* abstract versions; only for internal use (must match the subclassed
 * versions in DYNARRAY_TYPE_DECLARE) */
struct _element_abs {
    int used;
    int object; /* placeholder */
};



static inline struct _element_abs *get_nth_element(
        const struct _dynarray_abs *p_dynarray, int index, size_t element_size)
    __ATTR_CONST __NON_NULL;

static void set_element(struct _dynarray_abs *p_dynarray,
                        int index, const void *object, size_t obj_size,
                        size_t element_size) __NON_NULL;

static int find_free_element(const struct _dynarray_abs *p_dynarray,
                             size_t element_size) __ATTR_CONST __NON_NULL;

static int dynarray_grow(struct _dynarray_abs *p_dynarray,
                         const void *object, size_t obj_size,
                         size_t element_size) __NON_NULL;

static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
                          const void *object, size_t obj_size,
                          size_t element_size) __NON_NULL;


/*
 * Add an element to a dynarray, growing or reusing free elements as required.
 *
 * Receives the dynarray, the object and its size, and the size of elements
 * in this array. Returns the index of the newly added element. In case of
 * error (insufficient memory), returns -1.
 */
int dynarray_add(struct _dynarray_abs *p_dynarray, const void *object,
                 size_t obj_size, size_t element_size)
{
    assert(p_dynarray->used_count <= p_dynarray->len);

    if (p_dynarray->used_count == p_dynarray->len)
    {   /* array is full, must grow */
        return dynarray_grow(p_dynarray, object, obj_size, element_size);
    }
    else
    {   /* array has space, find a free element and reuse it */
        return dynarray_reuse(p_dynarray, object, obj_size, element_size);
    }
}



/*
 * Remove an element from a dynarray.
 *
 * Receives the dynarray, the index of the element to remove, and the size
 * of elements in this array. It is not an error to remove an element which
 * had already been removed. Returns 0 in case of success, or -1 in case of
 * error (invalid index).
 */
int dynarray_remove(struct _dynarray_abs *p_dynarray, int index,
                    size_t element_size)
{
    struct _element_abs *element;

    /* XXX: Do we want to shrink the array? Good because it saves memory;
     * bad because it changes the indexes for objects. Perhaps create a
     * separate operation to explicitly compact the array. */

    if (index >= p_dynarray->len)
        return -1;

    element = get_nth_element(p_dynarray, index, element_size);

    if (element->used)
    {
        element->used = 0;
        p_dynarray->used_count--;
    }

    return 0;
}



/*
 * Grow a dynarray and add a new element at the end.
 *
 * Receives the dynarray, the object and its size, and the size of elements for
 * this array. Returns the index of the newly added element. In case of error
 * (insufficient memory), returns -1.
 */
static int dynarray_grow(struct _dynarray_abs *p_dynarray,
                         const void *object, size_t obj_size,
                         size_t element_size)
{
    const int new_len = p_dynarray->len + 1;
    const int new_index = p_dynarray->len;
    void *elements = realloc(p_dynarray->elements, element_size * new_len);

    if (unlikely(elements == NULL))
        return -1;

    p_dynarray->elements = elements;
    p_dynarray->len = new_len;
    p_dynarray->used_count = new_len;

    set_element(p_dynarray, new_index, object, obj_size, element_size);

    return new_index;
}



/*
 * Reuse a free element in a dynarray.
 *
 * Receives the dynarray, the object and its size, and the size of elements for
 * this array. Returns the index where the element was stored.
 */
static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
                          const void *object, size_t obj_size,
                          size_t element_size)
{
    int first_free = find_free_element(p_dynarray, element_size);

    assert(first_free != -1);

    set_element(p_dynarray, first_free, object, obj_size, element_size);
    p_dynarray->used_count++;

    assert(p_dynarray->used_count <= p_dynarray->len);

    return first_free;
}



/*
 * Get the Nth element from a dynarray, given its index and the size of
 * the array's elements.
 */
static inline struct _element_abs *get_nth_element(
        const struct _dynarray_abs *p_dynarray, int index, size_t element_size)
{
    /* void * has alignment of 1 */
    return (struct _element_abs *)((void *)p_dynarray->elements
                                   + (index * element_size));
}



/*
 * Set the contents of an element in a dynarray.
 *
 * Receives the dynarray, the index of the element to set, the object and its
 * size.
 */
static void set_element(struct _dynarray_abs *p_dynarray,
                        int index, const void *object, size_t obj_size,
                        size_t element_size)
{
    struct _element_abs *element = get_nth_element(p_dynarray, index,
                                                   element_size);

    element->used = 1;
    memcpy(&element->object, object, obj_size);
}



/*
 * Find the first free element in an array.
 *
 * Returns the element's index or -1 if no free element was found.
 */
static int find_free_element(const struct _dynarray_abs *p_dynarray,
                             size_t element_size)
{
    int i;

    for (i = 0; i < p_dynarray->len; i++)
    {
        const struct _element_abs *element = get_nth_element(p_dynarray, i,
                                                             element_size);
        if (!element->used)
            return i;
    }

    return -1;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
