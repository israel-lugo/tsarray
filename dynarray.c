/*
 * dynarray - type-safe dynamic array library
 * Copyright 2012, 2015, 2016, 2017 Israel G. Lugo
 *
 * This file is part of dynarray.
 *
 * dynarray is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dynarray is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dynarray.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For suggestions, feedback or bug reports: israel.lugo@lugosys.com
 */



/*
 * dynarray.c - dynamic array module
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* get INT_MAX */
#include <limits.h>

/* get memcpy */
#include <string.h>

#include "dynarray.h"
#include "common.h"



/* Abstract item type definition. Used as a placeholder whenever we need to
 * manipulate items. The item type can be anything. */
struct _item_abs {
    int used;
    int object; /* placeholder */
};



static inline struct _item_abs *get_nth_item(
        const struct _item_abs *items, int index, size_t item_size)
    __ATTR_CONST __NON_NULL;

static void set_item(struct _item_abs *items, int index,
        const void *object, size_t obj_size, size_t item_size) __NON_NULL;

static int find_free_item(const struct _dynarray_abs *p_dynarray,
        size_t item_size) __ATTR_CONST __NON_NULL;

static int dynarray_append(struct _dynarray_abs *p_dynarray,
        const void *object, size_t obj_size, size_t item_size);

static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
        const void *object, size_t obj_size, size_t item_size);


/*
 * Add an item to a dynarray, growing or reusing free items as required.
 *
 * Receives the dynarray, an optional object, the object size for this
 * array, and the size of items in this array. Will find space for a new
 * item in the array, whether it be by reusing a free item or by growing
 * the array. If object is non-NULL, it will be copied to the new item.
 *
 * Returns the index of the newly added item in case of success, or a
 * negative error value in case of error.
 */
int dynarray_add(struct _dynarray_abs *p_dynarray, const void *object,
        size_t obj_size, size_t item_size)
{
    assert(p_dynarray->used_count <= p_dynarray->len);

    if (p_dynarray->used_count == p_dynarray->len)
    {   /* array is full, must grow */
        return dynarray_append(p_dynarray, object, obj_size, item_size);
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
 * had already been removed. Returns 0 in case of success, non-zero
 * otherwise.
 */
int dynarray_remove(struct _dynarray_abs *p_dynarray, int index,
        size_t item_size)
{
    struct _item_abs *item;

    /* XXX: Do we want to shrink the array? Good because it saves memory;
     * bad because it changes the indexes for objects. Perhaps create a
     * separate operation to explicitly compact the array. */

    if (index >= p_dynarray->len)
        return DYNARRAY_EINVAL;

    item = get_nth_item(p_dynarray->items, index, item_size);

    if (item->used)
    {
        item->used = 0;
        p_dynarray->used_count--;
    }

    return 0;
}



/*
 * Set a dynarray's minimum length.
 *
 * If the specified minimum length is greater than the current length, the
 * array is grown accordingly. Returns 0 in case of success, non-zero
 * otherwise.
 */
int dynarray_setminlen(struct _dynarray_abs *p_dynarray, int min_len,
        size_t item_size)
{
    if (unlikely(min_len < 0))
        return DYNARRAY_EINVAL;

    if (min_len <= p_dynarray->len)
        p_dynarray->min_len = min_len;
    else
    {   /* minimum length greater than current length; we must grow */
        int retval = dynarray_truncate(p_dynarray, min_len, item_size);

        if (unlikely(retval != 0))
            return retval;

        p_dynarray->min_len = min_len;
    }

    return 0;
}



/*
 * Compact a dynarray, removing all empty items.
 *
 * Removes all empty items ("holes") from a dynarray, by rearranging it so
 * all its used items are consecutive, then shrinking it to minimum size.
 * Items are guaranteed to remain in the same order.
 *
 * If the dynarray has too few empty items to be worth the work, nothing is
 * done; unless the force flag is non-zero, in which case will be compacted
 * anyway.
 *
 * Returns 0 in case of success, non-zero otherwise.
 */
int dynarray_compact(struct _dynarray_abs *p_dynarray, int force,
        size_t obj_size, size_t item_size)
{
    int hole_count;
    int hole_pct;

    assert(p_dynarray->len >= 0);
    assert(p_dynarray->used_count >= 0);

    /* skip empty arrays (avoid division by zero below) */
    if (p_dynarray->len == 0)
        return 0;

    hole_count = p_dynarray->len - p_dynarray->used_count;
    hole_pct = (hole_count * 100) / p_dynarray->len;

    assert(hole_count >= 0);
    assert(p_dynarray->items != NULL);

    if (hole_pct < 10 && (!force || hole_count == 0))
    {   /* less than 10%: very few or no holes */
        /* DO NOTHING */
    }
    else if (p_dynarray->used_count == 0)
    {   /* all holes, no used items at all */
        p_dynarray->len = 0;
        free(p_dynarray->items);
        p_dynarray->items = NULL;
    }
    else
    {   /* enough holes to care, but not all holes */
        struct _item_abs *items = p_dynarray->items;
        const int len = p_dynarray->len;
        int first_hole = INT_MAX;
        int i;
        int new_len;

        assert(p_dynarray->used_count < len);

        /*
         * Walk the array, looking for a hole. Remember its position.
         * Keep walking until we find a non-empty item. Move it to the
         * first hole. Now to the right of the first hole there must be a
         * hole: either there was a previously existing hole, or the
         * non-empty item which we just moved (turning it into a hole).
         */
        for (i = 0; i < len; i++)
        {
            struct _item_abs *item = get_nth_item(items, i, item_size);

            if (!item->used)
                first_hole = min(first_hole, i);
            else if (first_hole < i)
            {   /* non-empty item, and there's a previous hole to fill */
                set_item(items, first_hole, &item->object, obj_size, item_size);
                item->used = 0;
                /* there can only be empty items between first_hole and us;
                 * either there's another hole, or we're now the first hole */
                first_hole++;
            }
        }

        /* Non-empty items are now contiguous from the start; first_hole
         * marks the end of the data. We knew there were holes when we
         * entered, so first_hole can't be INT_MAX. */
        assert(first_hole == p_dynarray->used_count);

        /* make sure we don't shrink below configured minimum */
        new_len = max(first_hole, p_dynarray->min_len);
        if (new_len != len)
        {
            items = realloc(items, item_size * new_len);
            if (unlikely(items == NULL))
                return DYNARRAY_ENOMEM;

            p_dynarray->items = items;
            p_dynarray->len = new_len;
        }
    }

    return 0;
}



/*
 * Truncate a dynarray to a specific length.
 *
 * Receives the dynarray, the desired length and the size of items in this
 * array. If the dynarray is larger than the specified size, the extra data
 * is lost. If the dynarray is smaller than the specified size, it is
 * extended, and the extra items are all set to empty.
 *
 * Returns 0 in case of success, non-zero otherwise.
 */
int dynarray_truncate(struct _dynarray_abs *p_dynarray, int len,
        size_t item_size)
{
    assert(p_dynarray->len >= 0);
    assert(p_dynarray->min_len >= 0);

    if (unlikely(len < 0 || len < p_dynarray->min_len))
        return DYNARRAY_EINVAL;

    if (len == p_dynarray->len)
    {
        /* truncate to same size, do nothing */
    }
    else if (len == 0)
    {   /* clear the array */
        if (p_dynarray->items != NULL)
            free(p_dynarray->items);

        p_dynarray->len = 0;
        p_dynarray->used_count = 0;
        p_dynarray->items = NULL;
    }
    else
    {   /* grow or shrink */
        void *items = realloc(p_dynarray->items, len * item_size);

        if (unlikely(items == NULL))
            return DYNARRAY_ENOMEM;

        p_dynarray->items = items;

        if (len > p_dynarray->len)
        {   /* growing; initialize empty items */
            int i;

            for (i=p_dynarray->len; i<len; i++)
            {
                struct _item_abs *item = get_nth_item(items, i, item_size);

                item->used = 0;
            }
        }
        else
        {   /* shrinking; update used_count in case we eliminated used items */
            int used_count = 0;
            int i;

            for (i=0; i<len; i++)
            {
                const struct _item_abs *item = get_nth_item(items, i, item_size);

                if (item->used)
                    used_count++;
            }

            p_dynarray->used_count = used_count;
        }

        p_dynarray->len = len;
    }

    return 0;
}



/*
 * Grow a dynarray and add a new item at the end.
 *
 * Receives the dynarray, an optional object, the object size for this
 * array, and the size of items in this array. Will grow the array to make
 * space for a new item. If object is non-NULL, it will be copied to the
 * new item.
 *
 * Returns the index of the newly added item in case of success, or a
 * negative error value in case of error.
 */
static int dynarray_append(struct _dynarray_abs *p_dynarray,
        const void *object, size_t obj_size, size_t item_size)
{
    int old_len = p_dynarray->len;
    int retval;

    /*
     * TODO: This should be public. User should be able to add to the end
     * of the array, instead of at the first available hole. Problem is,
     * providing this possibility will limit us to only growing by 1 when
     * appending (otherwise user can't be expected to know exactly where
     * the end of the array is). This is actually a problem anyway, if we
     * want the len field to be of any meaning to the user. We have to
     * think whether we want to provide meaningful ordering or not.
     */

    /* protect from overflowing into negative lengths */
    if (!can_sadd(old_len, 1))
        return DYNARRAY_EOVERFLOW;

    /* XXX: Maybe we should grow in chunks, instead of 1 at a time. */
    retval = dynarray_truncate(p_dynarray, old_len + 1, item_size);
    if (retval != 0)
        return retval;

    if (object != NULL)
    {
        set_item(p_dynarray->items, old_len, object, obj_size, item_size);
        p_dynarray->used_count++;
    }

    return old_len;
}



/*
 * Reuse a free item in a dynarray.
 *
 * Receives the dynarray, an optional object, the object size for this
 * array, and the size of items in this array. Will find a free item which
 * can be reused. At least one free item MUST already exist. If object is
 * non-NULL, it will be copied to the new item.
 *
 * Returns the index where the item was stored.
 */
static int dynarray_reuse(struct _dynarray_abs *p_dynarray,
        const void *object, size_t obj_size, size_t item_size)
{
    int first_free = find_free_item(p_dynarray, item_size);

    assert(first_free != DYNARRAY_ENOENT);
    assert(p_dynarray->used_count < p_dynarray->len);

    if (object != NULL)
    {
        set_item(p_dynarray->items, first_free, object, obj_size, item_size);
        p_dynarray->used_count++;
    }

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
static void set_item(struct _item_abs *items, int index,
        const void *object, size_t obj_size, size_t item_size)
{
    struct _item_abs *item = get_nth_item(items, index, item_size);

    item->used = 1;
    memcpy(&item->object, object, obj_size);
}



/*
 * Find the first free item in an array.
 *
 * Returns the item's index or DYNARRAY_ENOENT if no free item was found.
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

    return DYNARRAY_ENOENT;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
