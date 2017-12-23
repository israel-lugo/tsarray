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

#include <stdlib.h>
#include <check.h>

/* get SIZE_MAX */
#include <stdint.h>

#include <tsarray.h>

#include "setupcheck.h"



/*
 * Test removing the only element of an array.
 */
START_TEST(test_remove_to_empty)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    int remove_result;

    append_seq_checked(a1, 10, 11);

    remove_result = intarray_remove(a1, 0);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(priv->len, 0);
}
END_TEST


/*
 * Test removing the first element of an array.
 */
START_TEST(test_remove_first)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const long start = -4;
    const long stop = 10;
    const unsigned long full_len = (unsigned long)(stop-start);
    int remove_result;
    unsigned long i;

    append_seq_checked(a1, start, stop);

    remove_result = intarray_remove(a1, 0);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(priv->len, full_len-1);

    /* check that remaining items are there, shifted 1 to the left */
    for (i=0; i<full_len-1; i++)
        ck_assert_int_eq(a1->items[i], start+(long)i+1);
}
END_TEST


/*
 * Test removing the last element of an array.
 */
START_TEST(test_remove_last)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const long start = -4;
    const long stop = 10;
    const unsigned long full_len = (unsigned long)(stop-start);
    int remove_result;
    unsigned long i;

    append_seq_checked(a1, start, stop);

    remove_result = intarray_remove(a1, (long)full_len-1);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(priv->len, full_len-1);

    /* check that remaining items are there, in the same place */
    for (i=0; i<full_len-1; i++)
        ck_assert_int_eq(a1->items[i], start+(long)i);
}
END_TEST


/*
 * Test trying to remove from an empty tsarray.
 */
START_TEST(test_remove_empty)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    int remove_result;

    remove_result = intarray_remove(a1, 0);
    ck_assert_int_eq(remove_result, TSARRAY_ENOENT);
    ck_assert_uint_eq(priv->len, 0);
}
END_TEST


/*
 * Test trying to remove non-existent item from non-empty tsarray.
 */
START_TEST(test_remove_noent)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int value = 10;
    int remove_result;

    append_seq_checked(a1, value, value+1);

    remove_result = intarray_remove(a1, 1);
    ck_assert_int_eq(remove_result, TSARRAY_ENOENT);

    /* make sure the existing item is still there */
    ck_assert_uint_eq(priv->len, 1);
    ck_assert_uint_ge(priv->capacity, priv->len);
    ck_assert_int_eq(a1->items[0], value);
}
END_TEST


/*
 * Test removing an item from the middle of a tsarray.
 *
 * The remaining items must still be there, in the correct order.
 */
START_TEST(test_remove_middle)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int stop = 20;
    const int remove_idx = stop/2;
    int remove_result;
    int i;

    append_seq_checked(a1, 0, stop);

    remove_result = intarray_remove(a1, remove_idx);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(priv->len, (unsigned long)stop-1);
    ck_assert_uint_ge(priv->capacity, priv->len);

    /* check that all remaining items are there, in right order */
    for (i=0; i<remove_idx; i++)
        ck_assert_int_eq(a1->items[i], i);
    for (i=remove_idx; i < stop-1; i++)
        ck_assert_int_eq(a1->items[i], i+1);
}
END_TEST


/*
 * Test removing many items from a tsarray.
 *
 * Append enough items to force resizing, then remove them again to check
 * for shrinking.
 */
START_TEST(test_remove_many)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int start = -1010;
    const int stop = 32010;
    const unsigned long full_len = (unsigned long)(stop-start);
    const unsigned long len_after_remove = 10;
    const unsigned long remove_count = full_len - len_after_remove;
    unsigned long full_capacity;
    unsigned long i;
    int remove_result;

    /* fill the array */
    append_seq_checked(a1, start, stop);
    ck_assert_uint_eq(priv->len, full_len);
    ck_assert_uint_ge(priv->capacity, priv->len);

    full_capacity = priv->capacity;

    for (i=0; i<remove_count; i++)
    {
        remove_result = intarray_remove(a1, 0);
        ck_assert_int_eq(remove_result, 0);
    }

    /* make sure len is correct, and array was shrunk */
    ck_assert_uint_eq(priv->len, len_after_remove);
    ck_assert_uint_lt(priv->capacity, full_capacity);
    ck_assert_uint_ge(priv->capacity, priv->len);
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("tsarray_remove");

    tc = tcase_with_a1_create("remove");

    tcase_add_test(tc, test_remove_to_empty);
    tcase_add_test(tc, test_remove_first);
    tcase_add_test(tc, test_remove_last);
    tcase_add_test(tc, test_remove_empty);
    tcase_add_test(tc, test_remove_noent);
    tcase_add_test(tc, test_remove_middle);
    tcase_add_test(tc, test_remove_many);

    suite_add_tcase(s, tc);

    return s;
}


int main(void)
{
    Suite *s = tsarray_suite();
    int number_failed = run_tests(s);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
