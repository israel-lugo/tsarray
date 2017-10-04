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


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <check.h>

#include <tsarray.h>

#include "setupcheck.h"


intarray *a1 = NULL;


/*
 * Append a sequence of ints to the specified intarray and perform checks.
 *
 * The sequence of ints ranges from start (inclusive) to stop (exclusive).
 *
 * Useful to fill arrays with junk inside unit tests. Checks the result,
 * length, capacity, and that the correct item was appended. Will fail the
 * test if any of these checks fail.
 */
void append_seq_checked(intarray *a, int start, int stop)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a;
    int i;

    for (i=start; i<stop; i++)
    {
        const size_t old_len = priv->len;
        int append_result = intarray_append(a, &i);
        ck_assert_int_eq(append_result, 0);

        ck_assert_uint_eq(priv->len, old_len+1);
        ck_assert_uint_ge(priv->capacity, priv->len);
        ck_assert_int_eq(a->items[old_len], i);
    }
}


/*
 * Create a new intarray and make a1 point to it.
 *
 * To be used as the setup for a checked test fixture.
 */
static void new_a1_array(void)
{
    a1 = intarray_new();
    ck_assert_ptr_ne(a1, NULL);
}


/*
 * Free the a1 intarray.
 *
 * To be used as the teardown for a checked test fixture.
 */
static void del_a1_array(void)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    ck_assert_int_eq((a1->items == NULL), (priv->capacity == 0));
    intarray_free(a1);
}


/*
 * Create a TCase with an intarray a1 test fixture.
 */
TCase *tcase_with_a1_create(const char *name)
{
    TCase *tc = tcase_create(name);
    tcase_add_checked_fixture(tc, new_a1_array, del_a1_array);

    return tc;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
