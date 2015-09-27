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

/* $Id: dynarray.c 267 2012-06-13 19:47:59Z capi $ */


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
        const struct _element_abs *elements, int index, size_t element_size)
    __ATTR_CONST __NON_NULL;

static void set_element(struct _element_abs *elements,
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

    element = get_nth_element(p_dynarray->elements, index, element_size);

    if (element->used)
    {
        element->used = 0;
        p_dynarray->used_count--;
    }

    return 0;
}



int dynarray_compact(struct _dynarray_abs *p_dynarray, int force,
                     size_t obj_size, size_t element_size)
{
    int hole_count;
    int hole_pct;

    /* skip empty arrays (avoid division by zero below) */
    if (p_dynarray->len == 0)
        return 0;

    hole_count = p_dynarray->len - p_dynarray->used_count;
    hole_pct = (hole_count * 100) / p_dynarray->len;

    assert(hole_count >= 0);
    assert(p_dynarray->elements != NULL);

    /*
     * TODO: More efficient algorithm for compacting:
     *
     * holes <- new FIFO()
     *
     * for i in 0..N:
     *   # Remember holes
     *   if array[i].is_hole:
     *     holes.enqueue(i)
     *
     *   # If we're not a hole and there are holes to fill, move ourselves
     *   elif !holes.empty:
     *     dst <- holes.dequeue()
     *     array[dst] <- array[i]
     *     array[i].is_hole <- True
     *
     * array.resize_to(N - hole_count)
     *
     * This uses N tests for emptiness and M copies, where M is the number
     * of non-empty nodes to the right of the first hole. We can compact
     * the array in O(N+M) time, assuming the FIFO is O(1) for enqueing and
     * dequeing, and that the resizing is also O(1), which should be true
     * of any sane realloc implementation when resizing to a smaller size.
     *
     * We use up to O(H) space, where H is the number of holes (we store
     * them in the FIFO).
     *
     * The FIFO can be implemented with a circular single-linked list.
     * We can make it O(1) for enqueue and dequeue, as long as we keep a
     * pointer to the last node. Just insert at the tail and pop from the
     * head.
     */

    if (hole_pct < 10 && (!force || hole_count == 0))
    {   /* less than 10%: very few or no holes */
        /* DO NOTHING */
    }
#if 0
    else if (hole_pct <= 50)
    {   /* TODO: many holes, between 11% and 50% */
        /* 1) linear search for first hole; when we find it, start looking
         * from the end for the first non-hole; move it to the hole; move
         * on -- problem: this changes order!
         *
         * 2) give every element some int position; holes have position =
         * INT_MAX or something; just quicksort the array, leaving all the
         * holes at the end
         */
    }
#endif
    else if (p_dynarray->used_count == 0)
    {   /* all holes, no used elements at all */
        p_dynarray->len = 0;
        free(p_dynarray->elements);
        p_dynarray->elements = NULL;
    }
    else
    {   /* mostly holes, more than 50% (but not all) */
        /* just copy the used objects to a new array */

        void *new_elements = malloc(element_size * p_dynarray->used_count);
        int i, j;

        if (unlikely(new_elements == NULL))
            return -1;

        j = 0;
        for (i = 0; i < p_dynarray->len; i++)
        {
            const struct _element_abs *element = get_nth_element(
                    p_dynarray->elements, i, element_size);

            if (!element->used)
                continue;

            assert(j < p_dynarray->used_count);

            set_element(new_elements, j, &element->object, obj_size, element_size);
            j++;
        }

        assert(j == p_dynarray->used_count);
        free(p_dynarray->elements);
        p_dynarray->elements = new_elements;
        p_dynarray->len = p_dynarray->used_count;
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

    set_element(p_dynarray->elements, new_index, object, obj_size, element_size);

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

    set_element(p_dynarray->elements, first_free, object, obj_size, element_size);
    p_dynarray->used_count++;

    assert(p_dynarray->used_count <= p_dynarray->len);

    return first_free;
}



/*
 * Get the Nth element from a dynarray's abstract element array, given its
 * index and the size of the array's elements.
 */
static inline struct _element_abs *get_nth_element(
        const struct _element_abs *elements, int index, size_t element_size)
{
    /* void * has alignment of 1 */
    return (struct _element_abs *)((void *)elements + (index * element_size));
}



/*
 * Set the contents of an element in a dynarray.
 *
 * Receives the dynarray's abstract element array, the index of the element
 * to set, the object and its size.
 */
static void set_element(struct _element_abs *elements,
                        int index, const void *object, size_t obj_size,
                        size_t element_size)
{
    struct _element_abs *element = get_nth_element(elements, index, element_size);

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
        const struct _element_abs *element = get_nth_element(p_dynarray->elements,
                                                             i, element_size);
        if (!element->used)
            return i;
    }

    return -1;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
