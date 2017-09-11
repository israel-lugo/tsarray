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

static void set_items(void *items, size_t index, const void *objects,
        size_t obj_size, size_t count);


/*
 * Create a new, empty, tsarray.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of error
 * while allocating memory.
 */
struct _tsarray_abs *tsarray_new(void)
{
    struct _tsarray_abs *new = malloc(sizeof(struct _tsarray_abs));

    if (unlikely(new == NULL))
        return NULL;

    new->len = 0;
    new->items = NULL;
    new->_priv.capacity = 0;

    return new;
}


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

    /* check if there's anything to change */
    if (old_len == new_len)
        return 0;

    /* asking for more objects than we can address? */
    if (unlikely(!can_size_mult(new_len, obj_size)))
        return TSARRAY_ENOMEM;

    /* Only change capacity if new_len is outside the hysteresis range
     * (i.e. there is no more free space, or too much). This avoids
     * overreacting to multiple append/remove patterns. */
    if (new_len > capacity || new_len < capacity/MIN_USAGE_RATIO)
    {
        assert(MIN_MARGIN <= SIZE_MAX - SIZE_MAX/MARGIN_RATIO);
        /* can never overflow, as long as the assert above is true */
        size_t margin = new_len/MARGIN_RATIO + MIN_MARGIN;
        void *new_items;

        /* if the margin makes us overflow, don't use it */
        if (unlikely(!can_size_add(new_len, margin)))
            margin = 0;

        capacity = new_len + margin;

        /* if we overflow converting to bytes, allocate less */
        if (unlikely(!can_size_mult(capacity, obj_size)))
        {   /* we know SIZE_MAX fits at least new_len*obj_size */
            capacity = SIZE_MAX/obj_size;
            assert(capacity >= new_len);
        }

        new_items = realloc(p_tsarray->items, capacity*obj_size);

        if (unlikely(new_items == NULL))
            return TSARRAY_ENOMEM;

        p_tsarray->items = new_items;
        p_tsarray->_priv.capacity = capacity;
    }

    p_tsarray->len = new_len;

    return 0;
}


/*
 * Create a tsarray from a copy of a C array.
 *
 * Receives a pointer to a source memory area (C array), the number of
 * items in the source, and the size of each item. Creates a new tsarray,
 * and fills it with a copy of the items from the source array.
 *
 * If src_len is zero, an empty tsarray will be created, as though with
 * tsarray_new(). The source array will not be read, and in particular it
 * may be NULL.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of
 * error.
 */
struct _tsarray_abs *tsarray_from_array(const void *src, size_t src_len,
        size_t obj_size)
{
    struct _tsarray_abs *new = tsarray_new();
    int retval;

    /* pass the error up */
    if (unlikely(new == NULL))
        return NULL;

    /* empty source array means empty tsarray */
    if (src_len == 0)
        return new;

    if (src == NULL)
    {   /* invalid args: src = NULL and src_len != 0 */
        goto _free_and_error;
    }

    retval = tsarray_resize(new, src_len, obj_size);
    if (unlikely(retval != 0))
    {   /* rollback and error out */
        goto _free_and_error;
    }

    assert(new->len == src_len);
    assert(new->len <= new->_priv.capacity);

    memcpy(new->items, src, src_len*obj_size);

    return new;

_free_and_error:
    tsarray_free(new);
    return NULL;
}


/*
 * Create a tsarray as a copy of an existing tsarray.
 *
 * Receives the source tsarray and the size of its items. Creates a new
 * tsarray, and fills it with a copy of the items from the source tsarray.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of
 * error.
 */
struct _tsarray_abs *tsarray_copy(const struct _tsarray_abs *p_tsarray_src,
        size_t obj_size)
{
    assert(p_tsarray_src->len <= p_tsarray_src->_priv.capacity);

    return tsarray_from_array(p_tsarray_src->items, p_tsarray_src->len, obj_size);
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
    assert(p_tsarray->len <= p_tsarray->_priv.capacity);

    set_items(p_tsarray->items, old_len, object, obj_size, 1);

    return 0;
}


/*
 * Extend a tsarray by appending objects from another tsarray.
 *
 * Receives the destination tsarray, the source tsarray, and the object
 * size. Appends a copy of all the objects in the source tsarray to the
 * dest tsarray. The source tsarray is not altered in any way.
 *
 * The source and destination tsarrays may be the same; the tsarray will be
 * extended with a copy of its own values.
 *
 * Returns zero in case of success, or a negative error value in case of
 * error.
 */
int tsarray_extend(struct _tsarray_abs *p_tsarray_dest,
        struct _tsarray_abs *p_tsarray_src, size_t obj_size)
{
    const size_t dest_len = p_tsarray_dest->len;
    const size_t src_len = p_tsarray_src->len;
    size_t new_len;
    int retval;

    if (unlikely(!can_size_add(dest_len, src_len)))
        return TSARRAY_EOVERFLOW;

    new_len = dest_len + src_len;

    retval = tsarray_resize(p_tsarray_dest, new_len, obj_size);
    if (unlikely(retval != 0))
        return retval;

    assert(p_tsarray_dest->len <= p_tsarray_dest->_priv.capacity);
    assert(p_tsarray_dest->len == dest_len + src_len);
    assert(p_tsarray_dest->items != NULL);

    /* XXX: Can we declare p_tsarray_src as pointer-to-const? Would be good
     * for the API contract. But tsarray_resize() may change dest->items,
     * and if dest == src, that means src->items will change too. Compiler
     * might assume that src->items is never altered and reuse an old copy.
     * Or might it? What are the pointer aliasing rules for differently
     * qualified pointers? In any case, this is only really relevant to the
     * set_items call, source arg. We _could_ just do something like
     * dest != src ? src->items : dest->items. */
    set_items(p_tsarray_dest->items, dest_len, p_tsarray_src->items, obj_size, src_len);

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
        char *item_to_rm = get_nth_item(p_tsarray->items, index, obj_size);

        memmove(item_to_rm, item_to_rm+obj_size, bytes_to_move);
    }

    return tsarray_resize(p_tsarray, old_len-1, obj_size);
}


/*
 * Free the memory occupied by a tsarray.
 *
 * This operation must be performed on a tsarray once it is no longer
 * necessary. Failure to do so will result in memory leaks.
 *
 * After this operation, the tsarray will be deallocated and invalid. It
 * must not be used for anything.
 */
void tsarray_free(struct _tsarray_abs *p_tsarray)
{
    /* items == NULL if and only if capacity == 0 */
    assert((p_tsarray->items == NULL) == (p_tsarray->_priv.capacity == 0));
    assert(p_tsarray->len <= p_tsarray->_priv.capacity);

    if (p_tsarray->items != NULL)
        free(p_tsarray->items);

    free(p_tsarray);
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
    /* can't use void for pointer arithmetic, it's an incomplete type
     * (also, char can alias anything; it's meant for this) */
    return ((char *)items + (index * obj_size));
}


/*
 * Set the contents of item(s) in a tsarray.
 *
 * Receives the tsarray's abstract item array, the index of the first item
 * to set, the array of objects, the object size, and the count of objects
 * to set.
 *
 * The count is not checked for validity. It MUST be within appropriate
 * bounds for writing to the items array (i.e. within array capacity) and
 * for reading from the source objects array.
 *
 * The memory area holding the source object MUST NOT overlap with the
 * destination memory area.
 */
static void set_items(void *items, size_t index, const void *objects,
        size_t obj_size, size_t count)
{
    char *dest = get_nth_item(items, index, obj_size);
    assert(can_size_mult(obj_size, count));
    const size_t bytes = obj_size*count;

    /* memory ranges don't overlap */
    assert(((char *)objects < dest && (char *)objects+bytes <= dest)
           || (dest < (char *)objects && dest+bytes <= (char *)objects));

    memcpy(dest, objects, obj_size*count);
}



/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
