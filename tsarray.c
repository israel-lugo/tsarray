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
 * tsarray.c - generic type-safe dynamic array module
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* get INT_MAX */
#include <limits.h>

/* get memcpy and memmove */
#include <string.h>

#include "tsarray.h"
#include "common.h"


/*
 * The array's capacity is calculated in tsarray_resize, according to the
 * following formula:
 *      capacity = len*(1 + 1/MARGIN_RATIO) + MIN_MARGIN
 *
 * MIN_MARGIN must be <= SIZE_MAX - SIZE_MAX/MARGIN_RATIO, to avoid integer
 * overflow in tsarray_resize.
 */
#define MARGIN_RATIO 8
#define MIN_MARGIN 4

/*
 * When the array's length drops below capacity/MIN_USAGE_RATIO, the array
 * is shrunk to save memory.
 */
#define MIN_USAGE_RATIO 2


static inline void *get_nth_item(const void *items, size_t index,
        size_t obj_size) __ATTR_CONST __NON_NULL;

static void set_item(void *items, size_t index, const void *object,
        size_t obj_size) __NON_NULL;


/*
 * Sets a tsarray's length, adjusting its capacity if necessary.
 *
 * Receives the tsarray, the new length, and the object size. Sets the
 * array's length, and resizes the array if necessary.
 *
 * If the new length is beyond the array's capacity, the array will be
 * enlarged. If the new length is below a minimum ratio of the capacity
 * (MIN_USAGE_RATIO), the array will be shrunk.
 *
 * If a resize is necessary, the new capacity will be calculated as:
 *      capacity = new_len*(1 + 1/MARGIN_RATIO) + MIN_MARGIN
 *
 * Returns zero in case of success, a negative error value otherwise.
 */
static int tsarray_resize(struct _tsarray_abs *p_tsarray, size_t new_len,
        size_t obj_size)
{
    const size_t old_len = p_tsarray->len;
    size_t capacity = p_tsarray->_priv.capacity;

    assert(old_len <= capacity);
    assert(p_tsarray->items != NULL || new_len == 0);

    /* check if there's anything to change */
    if (old_len == new_len)
        return 0;

    /* asking for more objects than we can address? */
    if (unlikely(new_len > SIZE_MAX/obj_size))
        return TSARRAY_ENOMEM;

    /* Only change capacity if new_len is outside the hysteresis range
     * (i.e. there is no more free space, or too much). This avoids
     * overreacting to multiple append/remove patterns. */
    if (new_len > capacity || new_len < capacity/MIN_USAGE_RATIO)
    {
        assert(MIN_MARGIN <= SIZE_MAX - SIZE_MAX/MARGIN_RATIO);
        /* can never overflow, as long as the assert above is true */
        const size_t margin = new_len/MARGIN_RATIO + MIN_MARGIN;
        void *new_items;

        if (unlikely(!can_size_add(new_len, margin)))
            return TSARRAY_EOVERFLOW;

        capacity = new_len + margin;
        new_items = realloc(p_tsarray->items, capacity);

        if (unlikely(new_items == NULL))
            return TSARRAY_ENOMEM;

        p_tsarray->items = new_items;
        p_tsarray->_priv.capacity = capacity;
    }

    p_tsarray->len = new_len;

    return 0;
}


/*
 * Append an object to the end of a tsarray.
 *
 * Receives the tsarray, an object and the object size for this array. Will
 * grow the array if necessary.
 *
 * Returns zero in case of success, or a negative error value in case of
 * error.
 */
int tsarray_append(struct _tsarray_abs *p_tsarray, const void *object,
        size_t obj_size)
{
    size_t old_len = p_tsarray->len;
    int retval;

    if (unlikely(!can_size_add(old_len, 1)))
        return TSARRAY_EOVERFLOW;

    retval = tsarray_resize(p_tsarray, old_len+1, obj_size);
    if (unlikely(retval != 0))
        return retval;

    /* there has to be room after a resize */
    assert(p_tsarray->len < p_tsarray->_priv.capacity);

    set_item(p_tsarray->items, old_len, object, obj_size);

    return 0;
}


/*
 * Remove one item from a tsarray.
 *
 * Receives a tsarray, the index of the item to remove, and the size of the
 * array's objects. Removes the item with the specified index and compacts
 * the array, by moving back any items with higher indices.
 *
 * Returns zero in case of success, or a negative error value otherwise.
 */
int tsarray_remove(struct _tsarray_abs *p_tsarray, int index, size_t obj_size)
{
    const size_t old_len = p_tsarray->len;

    if (unlikely(index >= old_len))
        return TSARRAY_ENOENT;

    assert(old_len <= p_tsarray->_priv.capacity);
    assert(old_len <= SIZE_MAX / obj_size);

    if (index < old_len-1)
    {   /* there's data to the right, need to move it left */
        const size_t bytes_to_move = (old_len - index - 1)*obj_size;
        void *item_to_rm = get_nth_item(p_tsarray->items, index, obj_size);

        memmove(item_to_rm, item_to_rm+obj_size, bytes_to_move);
    }

    return tsarray_resize(p_tsarray, old_len-1, obj_size);
}


/*
 * Get the address of an item in a tsarray.
 *
 * Get the Nth object from a tsarray's abstract item array, given its
 * index and the size of the array's objects.
 */
static inline void *get_nth_item(const void *items, size_t index,
        size_t obj_size)
{
    /* void * has alignment of 1 */
    return ((void *)items + (index * obj_size));
}


/*
 * Set the contents of an item in a tsarray.
 *
 * Receives the tsarray's abstract item array, the index of the item
 * to set, the object and its size.
 */
static void set_item(void *items, size_t index, const void *object,
        size_t obj_size)
{
    void *item = get_nth_item(items, index, obj_size);

    memcpy(item, object, obj_size);
}



/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
