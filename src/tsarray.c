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
    unsigned long capacity;     /* keep <= LONG_MAX, as indices are signed */
    unsigned long len;          /* likewise */
    unsigned long len_hint;    /* likewise */
    bool has_len_hint;
};



/*
 * The array's capacity is calculated in tsarray_resize, according to the
 * following formula:
 *      capacity = len*(1 + 1/MARGIN_RATIO) + MIN_MARGIN
 *
 * MIN_MARGIN must be <= LONG_MAX - LONG_MAX/MARGIN_RATIO, to avoid integer
 * overflow in tsarray_resize. See the calculation of margin there. We use
 * LONG_MAX (instead of ULONG_MAX) because we want to keep len and capacity
 * within signed long bounds.
 */
#define MARGIN_RATIO 8
#define MIN_MARGIN 4

/*
 * When the array's length drops below capacity/MIN_USAGE_RATIO, the array
 * is shrunk to save memory.
 */
#define MIN_USAGE_RATIO 2


/*
 * Estimated (assumed) standard deviation ratio for the length hint. Must
 * be >= 2 to make sense. If we take the hint to be the mean value, a
 * stddev of the entire mean throws off all the math.
 */
#define HINT_STDDEV_RATIO 3


static bool same_sign(int a, int b) __ATTR_CONST;

static inline void *get_nth_item(const void *items, long index,
        size_t obj_size) __ATTR_CONST __NON_NULL;

static unsigned long calc_new_capacity(size_t obj_size,
        unsigned long old_capacity, unsigned long new_len) __ATTR_CONST;

static unsigned long calc_new_capacity_with_hint(size_t obj_size,
        unsigned long old_capacity, unsigned long new_len,
        unsigned long len_hint) __ATTR_CONST;

static int tsarray_resize(struct _tsarray_priv *priv, unsigned long new_len) __NON_NULL;

static void set_items(void *items, long index, const void *objects,
        size_t obj_size, unsigned long count);



/*
 * Check if a and b are of the same sign.
 *
 * Returns true if and only if a and b are both positive, both negative, or
 * both zero.
 */
static bool same_sign(int a, int b)
{
    return (a > 0) == (b > 0) && (a < 0) == (b < 0);
}


/*
 * Get the number of items in a tsarray.
 */
unsigned long tsarray_len(const struct _tsarray_pub *tsarray)
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
    priv->len_hint = false;

    return &priv->pub;
}


/*
 * Create a new, empty, tsarray, with a length hint.
 *
 * Receives the size of the array's items, and a hint for the array's most
 * frequent expected length. Operations on the array will take the hint
 * into account, optimizating for the most frequent length.
 *
 * Returns a pointer to the newly created tsarray, or NULL in case of error
 * (invalid length hint, or unable to allocate memory).
 */
struct _tsarray_pub *tsarray_new_hint(size_t obj_size, unsigned long len_hint)
{
    if (unlikely(!is_valid_index(len_hint, obj_size)))
        return NULL;

    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray_new(obj_size);
    if (unlikely(priv == NULL))
        return NULL;

    priv->has_len_hint = true;
    priv->len_hint = len_hint;

    int retval = tsarray_resize(priv, 0);
    if (unlikely(retval != 0))
    {
        tsarray_free((struct _tsarray_pub *)priv);
        return NULL;
    }

    return &priv->pub;
}


/*
 * Create a new tsarray of the specified length.
 *
 * For internal use only. Receives the size of the array's items and the
 * desired array length, which MUST fit in a (signed) long index.
 *
 * Returns the private tsarray descriptor of newly created tsarray that
 * contains the specified amount of uninitialized items. In case of error,
 * returns NULL.
 */
static struct _tsarray_priv *_tsarray_new_of_len(size_t obj_size, unsigned long len)
{
    assert(ulong_fits_in_long(len));

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
 * Calculate the new capacity for a tsarray of a given new length.
 *
 * Receives the object size, the old capacity and the desired new length.
 * Returns the appropriate new capacity.
 *
 * The desired new length MUST be a valid index, i.e.:
 *  - it must fit in a signed long
 *  - it must be addressable in bytes (new_len*obj_size <= SIZE_MAX)
 */
static unsigned long calc_new_capacity(size_t obj_size,
        unsigned long old_capacity, unsigned long new_len)
{
    unsigned long margin;

    assert(ulong_fits_in_long(new_len));
    assert(new_len <= SIZE_MAX && can_size_mult(new_len, obj_size));

    /* Don't change capacity if new_len is within the hysteresis range
     * (i.e. there is still free space, and not too much). This avoids
     * overreacting to multiple append/remove patterns. */
    if (new_len <= old_capacity && new_len >= old_capacity/MIN_USAGE_RATIO)
        return old_capacity;

    assert(MIN_MARGIN <= LONG_MAX - LONG_MAX/MARGIN_RATIO);
    /* can never overflow, as long as the assert above is true */
    margin = new_len/MARGIN_RATIO + MIN_MARGIN;

    /* if the margin makes us overflow, don't use it */
    if (unlikely(!can_add_within_long(new_len, margin))
            || unlikely(new_len+margin > SIZE_MAX)
            || unlikely(!can_size_mult(new_len+margin, obj_size)))
        margin = 0;

    return new_len + margin;
}


/*
 * Calculate the new capacity for a tsarray with a length hint.
 *
 * Receives the object size, the old capacity, the desired new length and
 * the length hint. Returns the appropriate new capacity.
 *
 * Both the desired new length and the length hint MUST be valid indices:
 *  - they must fit in a signed long
 *  - they must be addressable in bytes (x*obj_size <= SIZE_MAX)
 */
static unsigned long calc_new_capacity_with_hint(size_t obj_size,
        unsigned long old_capacity, unsigned long new_len,
        unsigned long len_hint)
{
    /* We're using the three-sigma rule to create ranges around the
     * length hint, with different behaviors. Appropriate capacity is
     * chosen according to the range in which new_len falls. */

    assert(is_valid_index(new_len, obj_size));
    assert(is_valid_index(len_hint, obj_size));

    /* to keep things simple, we estimate standard deviation to be
     * 1/HINT_STDDEV_RATIO of the length hint */
    const unsigned long est_stddev = len_hint/HINT_STDDEV_RATIO;
    const unsigned long one_stddev_low = len_hint - est_stddev;
    const unsigned long one_stddev_high = ulong_add(len_hint, est_stddev);
    assert(2*est_stddev <= len_hint);
    const unsigned long two_stddev_low = len_hint - 2*est_stddev;

    /* do we need to resize at all? */
    if (old_capacity >= new_len                 /* enough free space */
        && old_capacity >= two_stddev_low       /* not too far below hint */
        && (old_capacity <= one_stddev_high     /* not too far above hint */
            || old_capacity-new_len <= est_stddev)) /* not wasting too much */
        return old_capacity;

    /* don't shrink too far below length hint */
    if (new_len < two_stddev_low)
        return two_stddev_low;

    if (new_len < one_stddev_low)
    {   /* two_stddev_low <= new_len < one_stddev_low: linear increase up to len_hint
         * slope = (len_hint-two_stddev_low)/(one_stddev_low-two_stddev_low)
         *       = (2*est_stddev) / est_stddev
         *       = 2
         * new_capacity = slope*(x1-x0) + y0
         *              = 2*(new_len-two_stddev_low) + two_stddev_low
         *              = 2*new_len - two_stddev_low
         *              = new_len - two_stddev_low + new_len
         *                  (for overflow protection)
         */
        const unsigned long new_capacity = new_len - two_stddev_low + new_len;
        assert(new_capacity >= new_len);
        assert(new_capacity <= len_hint);
        assert(is_valid_index(new_capacity, obj_size));
        return new_capacity;
    }

    if (new_len < len_hint)
    {   /* one_stddev_low <= new_len < len_hint: within one stddev of hint */
        return len_hint;
    }

    /* larger than the hint; just return new_len with an added margin */

    /* avoid overflowing with the margin in case new_len is really large */
    unsigned long new_capacity = ulong_add_capped_long(new_len, MIN_MARGIN);

    /* check if we can still address all bytes */
    if (unlikely(!is_valid_index(new_capacity, obj_size)))
        new_capacity = new_len;

    return new_capacity;
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
static int tsarray_resize(struct _tsarray_priv *priv, unsigned long new_len)
{
    const size_t obj_size = priv->obj_size;
    const unsigned long old_capacity = priv->capacity;
    unsigned long new_capacity;

    assert(ulong_fits_in_long(new_len));    /* must fit in signed long indices */
    assert(ulong_fits_in_long(old_len));
    assert(ulong_fits_in_long(old_capacity));
    assert(old_len <= old_capacity);

    /* asking for more objects than we can address? */
    if (unlikely(new_len > SIZE_MAX || !can_size_mult(new_len, obj_size)))
        return TSARRAY_ENOMEM;

    new_capacity = priv->has_len_hint
        ?  calc_new_capacity_with_hint(obj_size, old_capacity, new_len,
                                       priv->len_hint)
        : calc_new_capacity(obj_size, old_capacity, new_len);

    if (new_capacity != old_capacity)
    {
        void *new_items = realloc(priv->pub.items, new_capacity*obj_size);

        /* if we asked for 0 bytes, realloc may legitimately return NULL */
        if (unlikely(new_items == NULL && new_capacity != 0))
            return TSARRAY_ENOMEM;

        priv->pub.items = new_items;
        priv->capacity = new_capacity;
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
struct _tsarray_pub *tsarray_from_array(const void *src, unsigned long src_len,
        size_t obj_size)
{
    struct _tsarray_priv *priv;
    struct _tsarray_pub *pub;

    /* must fit in signed long indices */
    if (!ulong_fits_in_long(src_len))
        return NULL;

    priv = _tsarray_new_of_len(obj_size, src_len);
    pub = &priv->pub;

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

    /* no need to check for overflow in src_len*obj_size; we were able to
     * allocate at least that, in _tsarray_new_of_len */
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
        long start, long stop, long step)
{
    const struct _tsarray_priv *src_priv = (const struct _tsarray_priv *)src_tsarray;
    const size_t obj_size = src_priv->obj_size;
    const long lo_bound = min(start, stop);

    /* make sure we don't overflow converting len to long */
    assert(ulong_fits_in_long(src_priv->len));
    const long hi_bound = min(max(start, stop), (long)src_priv->len);

    /* TODO: Support negative indices, "counting from last", like Python */

    assert(src_priv->len <= src_priv->capacity);

    /* zero step makes no sense */
    if (step == 0)
        return NULL;

    /* shortcircuit emty cases */
    if (start == stop                          /* requested empty slice */
            || (start < stop) != (step > 0)    /* direction contradicts step */
            || lo_bound >= (long)src_priv->len) /* lower bound beyond array */
        return tsarray_new(obj_size);

    assert(lo_bound < hi_bound);
    assert(hi_bound <= (long)src_priv->len);

    if (step == 1)
    {   /* simple case: straightforward cut */
        const char *first = get_nth_item(src_tsarray->items, lo_bound, obj_size);
        const unsigned long slice_len = (unsigned long)(hi_bound - lo_bound);

        return tsarray_from_array(first, slice_len, obj_size);
    }
    else
    {   /* stepping over items, or going backwards */
        const unsigned long slice_len = (unsigned long)((hi_bound - lo_bound - 1)/labs(step)) + 1;
        struct _tsarray_priv *slice_priv = _tsarray_new_of_len(obj_size, slice_len);
        /* when going backwards, user may tell us to start beyond the array */
        const long real_start = min(start, (long)src_priv->len-1);
        long i;

        assert(ulong_fits_in_long(slice_len));
        assert(can_long_mult((long)slice_len-1, step));

        for (i=0; i<(long)slice_len; i++)
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
 * Scan a tsarray, looking for the smallest or largest item.
 *
 * Receives the array, a comparison function, a generic argument to pass
 * to the comparison function for context, and the direction in which to
 * scan. If direction is negative, look for the smallest item. If direction
 * is positive, look for the largest item. Direction must not be zero.
 *
 * Returns a pointer to the chosen item, or NULL if the array is empty.
 */
static void *minmax_scan(const struct _tsarray_pub *tsarray,
        int (*cmp)(const void *a, const void *b, void *arg),
        void *arg, int direction)
{
    const struct _tsarray_priv *priv = (const struct _tsarray_priv *)tsarray;
    const unsigned long len = priv->len;
    const size_t obj_size = priv->obj_size;
    char *candidate = (char *)(tsarray->items);
    long i;

    assert(direction != 0);

    if (len == 0)
        return NULL;

    for (i=1; i<(long)len; i++)
    {
        char *item = get_nth_item(tsarray->items, i, obj_size);
        int diff = cmp(item, candidate, arg);
        if (same_sign(direction, diff))
            candidate = item;
    }

    return candidate;
}


/*
 * Return a pointer to smallest item in a tsarray.
 *
 * Receives the tsarray, a comparison function, and a generic argument that
 * will be passed to the comparison function for context.
 *
 * The comparison function should return a number less than, equal to, or
 * greater than zero if the first argument is, respectively, less than,
 * equal to, or greater than the second.
 *
 * Returns a pointer to the smallest item, or NULL if the array is empty.
 */
void *tsarray_min(const struct _tsarray_pub *tsarray,
        int (*cmp)(const void *a, const void *b, void *arg), void *arg)
{
    return minmax_scan(tsarray, cmp, arg, -1);
}


/*
 * Return a pointer to largest item in a tsarray.
 *
 * Receives the tsarray, a comparison function, and a generic argument that
 * will be passed to the comparison function for context.
 *
 * The comparison function should return a number less than, equal to, or
 * greater than zero if the first argument is, respectively, less than,
 * equal to, or greater than the second.
 *
 * Returns a pointer to the largest item, or NULL if the array is empty.
 */
void *tsarray_max(const struct _tsarray_pub *tsarray,
        int (*cmp)(const void *a, const void *b, void *arg), void *arg)
{
    return minmax_scan(tsarray, cmp, arg, 1);
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
    const unsigned long old_len = priv->len;
    int retval;

    assert(ulong_fits_in_long(old_len));
    assert(ulong_fits_in_long(priv->capacity));

    if (unlikely(!can_add_within_long(old_len, 1)))
        return TSARRAY_EOVERFLOW;

    retval = tsarray_resize(priv, old_len+1);
    if (unlikely(retval != 0))
        return retval;

    /* there has to be room after a resize */
    assert(priv->len <= priv->capacity);

    set_items(tsarray->items, (long)old_len, object, priv->obj_size, 1);

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
    const unsigned long dest_len = priv_dest->len;
    const unsigned long src_len = priv_src->len;
    unsigned long new_len;
    int retval;

    assert(ulong_fits_in_long(src_len));
    assert(ulong_fits_in_long(dest_len));

    /* arrays must be of the same thing (or at least same object size) */
    if (unlikely(priv_dest->obj_size != priv_src->obj_size))
        return TSARRAY_EINVAL;

    if (unlikely(!can_add_within_long(dest_len, src_len)))
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
    set_items(tsarray_dest->items, (long)dest_len, tsarray_src->items, obj_size, src_len);

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
int tsarray_remove(struct _tsarray_pub *tsarray, long index)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)tsarray;
    const size_t obj_size = priv->obj_size;
    const unsigned long old_len = priv->len;

    assert(ulong_fits_in_long(old_len));

    /* we don't allow negative indices; will do sometime, a la Python */
    if (unlikely(index < 0))
        return TSARRAY_EINVAL;

    if (unlikely(index >= (long)old_len))
        return TSARRAY_ENOENT;

    assert(old_len > 0);
    assert(old_len <= priv->capacity);
    assert(old_len <= SIZE_MAX / obj_size);

    if ((unsigned long)index < old_len-1)
    {   /* there's data to the right, need to move it left */
        const size_t bytes_to_move = (old_len - (unsigned long)index - 1)*obj_size;
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
 * index and the size of the array's objects. index MUST be positive.
 */
static inline void *get_nth_item(const void *items, long index,
        size_t obj_size)
{
    /* TODO: Implement negative indices, a la Python */
    assert(index >= 0);

    /* can't use void for pointer arithmetic, it's an incomplete type
     * (also, char can alias anything; it's meant for this) */
    return ((char *)items + ((unsigned long)index * obj_size));
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
static void set_items(void *items, long index, const void *objects,
        size_t obj_size, unsigned long count)
{
    char *dest = get_nth_item(items, index, obj_size);
    assert(count <= SIZE_MAX && can_size_mult(obj_size, count));
    const size_t bytes = obj_size*count;

    /* memory ranges don't overlap */
    assert(((char *)objects < dest && (char *)objects+bytes <= dest)
           || (dest < (char *)objects && dest+bytes <= (char *)objects));

    memcpy(dest, objects, bytes);
}



/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
