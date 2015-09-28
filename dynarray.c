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


/* Abstract item type definition. Used as a placeholder whenever we need to
 * manipulate items. The item type can be anything. */
_ITEM_TYPE_DECL(_item_abs, int);



static inline struct _item_abs *get_nth_item(
        const struct _item_abs *items, int index, size_t item_size)
    __ATTR_CONST __NON_NULL;

static void set_item(struct _item_abs *items,
                        int index, const void *object, size_t obj_size,
                        size_t item_size) __NON_NULL;

static int find_free_item(const struct _dynarray_abs *p_dynarray,
                             size_t item_size) __ATTR_CONST __NON_NULL;

static int dynarray_grow(struct _dynarray_abs *p_dynarray,
                         const void *object, size_t obj_size,
                         size_t item_size) __NON_NULL;

static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
                          const void *object, size_t obj_size,
                          size_t item_size) __NON_NULL;


/*
 * Add an item to a dynarray, growing or reusing free items as required.
 *
 * Receives the dynarray, the object and its size, and the size of items
 * in this array. Returns the index of the newly added item. In case of
 * error (insufficient memory), returns -1.
 */
int dynarray_add(struct _dynarray_abs *p_dynarray, const void *object,
                 size_t obj_size, size_t item_size)
{
    assert(p_dynarray->used_count <= p_dynarray->len);

    if (p_dynarray->used_count == p_dynarray->len)
    {   /* array is full, must grow */
        return dynarray_grow(p_dynarray, object, obj_size, item_size);
    }
    else
    {   /* array has space, find a free item and reuse it */
        return dynarray_reuse(p_dynarray, object, obj_size, item_size);
    }
}



/*
 * Remove an item from a dynarray.
 *
 * Receives the dynarray, the index of the item to remove, and the size
 * of items in this array. It is not an error to remove an item which
 * had already been removed. Returns 0 in case of success, or -1 in case of
 * error (invalid index).
 */
int dynarray_remove(struct _dynarray_abs *p_dynarray, int index,
                    size_t item_size)
{
    struct _item_abs *item;

    /* XXX: Do we want to shrink the array? Good because it saves memory;
     * bad because it changes the indexes for objects. Perhaps create a
     * separate operation to explicitly compact the array. */

    if (index >= p_dynarray->len)
        return -1;

    item = get_nth_item(p_dynarray->items, index, item_size);

    if (item->used)
    {
        item->used = 0;
        p_dynarray->used_count--;
    }

    return 0;
}



int dynarray_compact(struct _dynarray_abs *p_dynarray, int force,
                     size_t obj_size, size_t item_size)
{
    int hole_count;
    int hole_pct;

    /* skip empty arrays (avoid division by zero below) */
    if (p_dynarray->len == 0)
        return 0;

    hole_count = p_dynarray->len - p_dynarray->used_count;
    hole_pct = (hole_count * 100) / p_dynarray->len;

    assert(hole_count >= 0);
    assert(p_dynarray->items != NULL);

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
         * 2) give every item some int position; holes have position =
         * INT_MAX or something; just quicksort the array, leaving all the
         * holes at the end
         */
    }
#endif
    else if (p_dynarray->used_count == 0)
    {   /* all holes, no used items at all */
        p_dynarray->len = 0;
        free(p_dynarray->items);
        p_dynarray->items = NULL;
    }
    else
    {   /* mostly holes, more than 50% (but not all) */
        /* just copy the used objects to a new array */

        void *new_items = malloc(item_size * p_dynarray->used_count);
        int i, j;

        if (unlikely(new_items == NULL))
            return -1;

        j = 0;
        for (i = 0; i < p_dynarray->len; i++)
        {
            const struct _item_abs *item = get_nth_item(
                    p_dynarray->items, i, item_size);

            if (!item->used)
                continue;

            assert(j < p_dynarray->used_count);

            set_item(new_items, j, &item->object, obj_size, item_size);
            j++;
        }

        assert(j == p_dynarray->used_count);
        free(p_dynarray->items);
        p_dynarray->items = new_items;
        p_dynarray->len = p_dynarray->used_count;
    }

    return 0;
}




/*
 * Grow a dynarray and add a new item at the end.
 *
 * Receives the dynarray, the object and its size, and the size of items for
 * this array. Returns the index of the newly added item. In case of error
 * (insufficient memory), returns -1.
 */
static int dynarray_grow(struct _dynarray_abs *p_dynarray,
                         const void *object, size_t obj_size,
                         size_t item_size)
{
    const int new_len = p_dynarray->len + 1;
    const int new_index = p_dynarray->len;
    void *items = realloc(p_dynarray->items, item_size * new_len);

    if (unlikely(items == NULL))
        return -1;

    p_dynarray->items = items;
    p_dynarray->len = new_len;
    p_dynarray->used_count = new_len;

    set_item(p_dynarray->items, new_index, object, obj_size, item_size);

    return new_index;
}



/*
 * Reuse a free item in a dynarray.
 *
 * Receives the dynarray, the object and its size, and the size of items for
 * this array. Returns the index where the item was stored.
 */
static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
                          const void *object, size_t obj_size,
                          size_t item_size)
{
    int first_free = find_free_item(p_dynarray, item_size);

    assert(first_free != -1);

    set_item(p_dynarray->items, first_free, object, obj_size, item_size);
    p_dynarray->used_count++;

    assert(p_dynarray->used_count <= p_dynarray->len);

    return first_free;
}



/*
 * Get the Nth item from a dynarray's abstract item array, given its
 * index and the size of the array's items.
 */
static inline struct _item_abs *get_nth_item(
        const struct _item_abs *items, int index, size_t item_size)
{
    /* void * has alignment of 1 */
    return (struct _item_abs *)((void *)items + (index * item_size));
}



/*
 * Set the contents of an item in a dynarray.
 *
 * Receives the dynarray's abstract item array, the index of the item
 * to set, the object and its size.
 */
static void set_item(struct _item_abs *items,
                        int index, const void *object, size_t obj_size,
                        size_t item_size)
{
    struct _item_abs *item = get_nth_item(items, index, item_size);

    item->used = 1;
    memcpy(&item->object, object, obj_size);
}



/*
 * Find the first free item in an array.
 *
 * Returns the item's index or -1 if no free item was found.
 */
static int find_free_item(const struct _dynarray_abs *p_dynarray,
                             size_t item_size)
{
    int i;

    for (i = 0; i < p_dynarray->len; i++)
    {
        const struct _item_abs *item = get_nth_item(p_dynarray->items,
                                                             i, item_size);
        if (!item->used)
            return i;
    }

    return -1;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
