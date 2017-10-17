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

/* get abs() */
#include <stdlib.h>

/* get memcpy and memmove */
#include <string.h>

#include "tsarray.h"
#include "common.h"


/*
 * Private version of the tsarray. This contains the public version that
 * the user knows about, and gives to us.
 *
 * C99 allows us to access the same object internally through these
 * different pointers, since the public version is contained by the private
 * one, and (by section 6.5) an aggregate may alias a pointer to one of its
 * members.
 *
 * The public object need not be the first member in the private one.
 * However, this makes it easier to get the private pointer from the public
 * one: we just need to do a cast. Otherwise, we would need a get_priv()
 * that subtracts offsetof(struct _tsarray_priv, pub) from its argument
 * (cast to char *), then casts the result to struct _tsarray_priv *.
 */
struct _tsarray_priv {
    struct _tsarray_pub pub;    /* MUST be the first member (read above) */
    size_t obj_size;
    size_t capacity;
    size_t len;
};



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

static int tsarray_resize(struct _tsarray_priv *priv, size_t new_len) __NON_NULL;

static void set_items(void *items, size_t index, const void *objects,
        size_t obj_size, size_t count);


/*
 * Get the number of items in a tsarray.
 */
size_t tsarray_len(const struct _tsarray_pub *tsarray)
{
    return ((const struct _tsarray_priv *)tsarray)->len;
}


/*
 * Create a new, empty, tsarray.
 *
 * Receives the size of the array's items. Returns a pointer to the newly
 * created tsarray, or NULL in case of error while allocating memory.
 */
struct _tsarray_pub *tsarray_new(size_t obj_size)
{
    struct _tsarray_priv *priv = malloc(sizeof(struct _tsarray_priv));

    if (unlikely(priv == NULL))
        return NULL;

    priv->pub.items = NULL;
    priv->obj_size = obj_size;
    priv->capacity = 0;
    priv->len = 0;

    return &priv->pub;
}


/*
 * Create a new tsarray of the specified length.
 *
 * For internal use only. Receives the size of the array's items, and the
 * desired array length. Returns the private tsarray descriptor of newly
 * created tsarray that contains the specified amount of uninitialized
 * items. In case of error, returns NULL.
 */
static struct _tsarray_priv *_tsarray_new_of_len(size_t obj_size, size_t len)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray_new(obj_size);
    int retval;

    if (unlikely(priv == NULL))
        return NULL;

    retval = tsarray_resize(priv, len);
    if (unlikely(retval != 0))
    {
        tsarray_free((struct _tsarray_pub *)priv);
        return NULL;
    }

    return priv;
}


/*
 * Sets a tsarray's length, adjusting its capacity if necessary.
 *
 * Receives a private tsarray descriptor and the new length. Sets the
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
static int tsarray_resize(struct _tsarray_priv *priv, size_t new_len)
{
    const size_t old_len = priv->len;
    const size_t obj_size = priv->obj_size;
    size_t capacity = priv->capacity;

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

        new_items = realloc(priv->pub.items, capacity*obj_size);

        if (unlikely(new_items == NULL))
            return TSARRAY_ENOMEM;

        priv->pub.items = new_items;
        priv->capacity = capacity;
    }

    priv->len = new_len;

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
struct _tsarray_pub *tsarray_from_array(const void *src, size_t src_len,
        size_t obj_size)
{
    struct _tsarray_priv *priv = _tsarray_new_of_len(obj_size, src_len);
    struct _tsarray_pub *pub = &priv->pub;

    /* pass the error up */
    if (unlikely(priv == NULL))
        return NULL;

    /* empty source array means empty tsarray */
    if (src_len == 0)
        return pub;

    if (src == NULL)
    {   /* invalid args: src = NULL and src_len != 0 */
        tsarray_free(pub);
        return NULL;
    }

    assert(priv->len == src_len);
    assert(priv->len <= priv->capacity);

    memcpy(pub->items, src, src_len*obj_size);

    return pub;
}


/*
 * Create a tsarray as a copy of an existing tsarray.
 *
 * Receives the source tsarray. Creates a new tsarray, and fills it with a
 * copy of the items from the source tsarray.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of
 * error.
 */
struct _tsarray_pub *tsarray_copy(const struct _tsarray_pub *src_tsarray)
{
    const struct _tsarray_priv *priv = (const struct _tsarray_priv *)src_tsarray;

    assert(priv->len <= priv->capacity);

    return tsarray_from_array(src_tsarray->items, priv->len, priv->obj_size);
}


/*
 * Create a tsarray as a slice of an existing tsarray.
 *
 * Receives the source tsarray, the slice start and stop indexes, and the
 * step value. step may be positive (to slice forward) or negative (to
 * slice backwards), but not zero.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of
 * error.
 */
struct _tsarray_pub *tsarray_slice(const struct _tsarray_pub *src_tsarray,
        size_t start, size_t stop, int step)
{
    const struct _tsarray_priv *src_priv = (const struct _tsarray_priv *)src_tsarray;
    const size_t obj_size = src_priv->obj_size;
    const size_t lo_bound = min(start, stop);
    const size_t hi_bound = min(max(start, stop), src_priv->len);

    assert(src_priv->len <= src_priv->capacity);

    /* zero step makes no sense */
    if (step == 0)
        return NULL;

    /* shortcircuit emty cases */
    if (start == stop                          /* requested empty slice */
            || (start < stop) != (step > 0)    /* direction contradicts step */
            || lo_bound >= src_priv->len)      /* lower bound beyond array */
        return tsarray_new(obj_size);

    assert(lo_bound < hi_bound);
    assert(hi_bound <= src_priv->len);

    if (step == 1)
    {   /* simple case: straightforward cut */
        const char *first = get_nth_item(src_tsarray->items, lo_bound, obj_size);
        const size_t slice_len = hi_bound - lo_bound;

        return tsarray_from_array(first, slice_len, obj_size);
    }
    else
    {   /* stepping over items, or going backwards */
        const size_t slice_len = 1 + ((hi_bound - lo_bound - 1)/abs(step));
        struct _tsarray_priv *slice_priv = _tsarray_new_of_len(obj_size, slice_len);
        /* when going backwards, user may tell us to start beyond the array */
        const size_t real_start = min(start, src_priv->len-1);

        for (size_t i=0; i<slice_len; i++)
        {
            const char *src = get_nth_item(src_tsarray->items, real_start + i*step, obj_size);
            char *dest = get_nth_item(slice_priv->pub.items, i, obj_size);

            memcpy(dest, src, obj_size);
        }

        return &slice_priv->pub;
    }
    /* UNREACHABLE */
}


/*
 * Append an object to the end of a tsarray.
 *
 * Receives the tsarray, and a pointer to an object. Will grow the array if
 * necessary.
 *
 * Returns zero in case of success, or a negative error value in case of
 * error.
 */
int tsarray_append(struct _tsarray_pub *tsarray, const void *object)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray;
    const size_t old_len = priv->len;
    int retval;

    if (unlikely(!can_size_add(old_len, 1)))
        return TSARRAY_EOVERFLOW;

    retval = tsarray_resize(priv, old_len+1);
    if (unlikely(retval != 0))
        return retval;

    /* there has to be room after a resize */
    assert(priv->len <= priv->capacity);

    set_items(tsarray->items, old_len, object, priv->obj_size, 1);

    return 0;
}


/*
 * Extend a tsarray by appending objects from another tsarray.
 *
 * Receives the destination tsarray and the source tsarray. Appends a copy
 * of all the objects in the source tsarray to the destination tsarray. The
 * source tsarray is not altered in any way.
 *
 * The source and destination tsarrays may be the same; the tsarray will be
 * extended with a copy of its own values.
 *
 * Returns zero in case of success, or a negative error value in case of
 * error.
 */
int tsarray_extend(struct _tsarray_pub *tsarray_dest,
        struct _tsarray_pub *tsarray_src)
{
    struct _tsarray_priv *priv_dest = (struct _tsarray_priv *)tsarray_dest;
    struct _tsarray_priv *priv_src = (struct _tsarray_priv *)tsarray_src;
    const size_t obj_size = priv_src->obj_size;
    const size_t dest_len = priv_dest->len;
    const size_t src_len = priv_src->len;
    size_t new_len;
    int retval;

    assert(priv_dest->obj_size == priv_src->obj_size);

    if (unlikely(!can_size_add(dest_len, src_len)))
        return TSARRAY_EOVERFLOW;

    new_len = dest_len + src_len;

    retval = tsarray_resize(priv_dest, new_len);
    if (unlikely(retval != 0))
        return retval;

    assert(priv_dest->len <= priv_dest->capacity);
    assert(priv_dest->len == dest_len + src_len);
    assert(tsarray_dest->items != NULL);

    /* XXX: Can we declare tsarray_src as pointer-to-const? Would be good
     * for the API contract. But tsarray_resize() may change dest->items,
     * and if dest == src, that means src->items will change too. Compiler
     * might assume that src->items is never altered and reuse an old copy.
     * Or might it? Pointers differing only in qualification are allowed to
     * alias. In any case, this is only really relevant to the set_items
     * call, source arg. We _could_ just do something like dest != src ?
     * src->items : dest->items. */
    set_items(tsarray_dest->items, dest_len, tsarray_src->items, obj_size, src_len);

    return 0;
}


/*
 * Remove one item from a tsarray.
 *
 * Receives a tsarray and the index of the item to remove. Removes the item
 * with the specified index and compacts the array, by moving back any
 * items with higher indices.
 *
 * Returns zero in case of success, or a negative error value otherwise.
 */
int tsarray_remove(struct _tsarray_pub *tsarray, int index)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray;
    const size_t obj_size = priv->obj_size;
    const size_t old_len = priv->len;

    if (unlikely(index >= old_len))
        return TSARRAY_ENOENT;

    assert(old_len <= priv->capacity);
    assert(old_len <= SIZE_MAX / obj_size);

    if (index < old_len-1)
    {   /* there's data to the right, need to move it left */
        const size_t bytes_to_move = (old_len - index - 1)*obj_size;
        char *item_to_rm = get_nth_item(tsarray->items, index, obj_size);

        memmove(item_to_rm, item_to_rm+obj_size, bytes_to_move);
    }

    return tsarray_resize(priv, old_len-1);
}


/*
 * Free the memory occupied by a tsarray.
 *
 * This operation must be performed on a tsarray once it is no longer
 * necessary. Failure to do so will result in memory leaks.
 *
 * After this operation, the tsarray will be deallocated and invalid. It
 * may not be used for anything.
 */
void tsarray_free(struct _tsarray_pub *tsarray)
{
    /* items == NULL if and only if capacity == 0 */
    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray;

    assert((tsarray->items == NULL) == (priv->capacity == 0));
    assert(priv->len <= priv->capacity);

    if (tsarray->items != NULL)
        free(tsarray->items);

    free(priv);
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

    memcpy(dest, objects, bytes);
}



/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
