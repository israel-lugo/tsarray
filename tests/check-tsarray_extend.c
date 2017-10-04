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
 * Test extending a tsarray with elements from another one.
 */
START_TEST(test_extend)
{
    intarray *a2 = intarray_new();
    struct _tsarray_priv *priv1 = (struct _tsarray_priv *)a1;
    struct _tsarray_priv *priv2 = (struct _tsarray_priv *)a2;
    const int a1stop = 10;
    const int a2stop = 60;
    int *a2_items;
    int extend_result;
    int i;

    ck_assert_ptr_ne(a2, NULL);

    /* a1 = [0..a1stop); a2 = [a1stop..a2stop) */
    append_seq_checked(a1, 0, a1stop);
    append_seq_checked(a2, a1stop, a2stop);
    a2_items = a2->items;

    extend_result = intarray_extend(a1, a2);
    ck_assert_int_eq(extend_result, 0);

    /* check a2 wasn't changed, then free it */
    ck_assert_uint_eq(priv2->len, a2stop-a1stop);
    ck_assert_ptr_eq(a2->items, a2_items);
    for (i=0; i<a2stop-a1stop; i++)
        ck_assert_int_eq(a2->items[i], i+a1stop);
    intarray_free(a2);

    /* check a1 was extended with the contents of a2 */
    ck_assert_uint_eq(priv1->len, a2stop);
    ck_assert_uint_ge(priv1->capacity, priv1->len);

    for (i=0; i<a2stop; i++)
        ck_assert_int_eq(a1->items[i], i);
}
END_TEST


/*
 * Test extending a tsarray with an empty tsarray.
 */
START_TEST(test_extend_with_empty)
{
    intarray *a2 = intarray_new();
    struct _tsarray_priv *priv1 = (struct _tsarray_priv *)a1;
    const int value = 33;
    int extend_result;

    append_seq_checked(a1, value, value+1);

    extend_result = intarray_extend(a1, a2);
    ck_assert_int_eq(extend_result, 0);

    intarray_free(a2);

    ck_assert_uint_eq(priv1->len, 1);
    ck_assert_uint_ge(priv1->capacity, priv1->len);
    ck_assert_int_eq(a1->items[0], value);
}
END_TEST


/*
 * Test extending an empty tsarray with a non-empty one.
 */
START_TEST(test_extend_empty)
{
    struct _tsarray_priv *priv1 = (struct _tsarray_priv *)a1;
    intarray *a2 = intarray_new();
    const int value = 33;
    int extend_result;

    append_seq_checked(a2, value, value+1);

    extend_result = intarray_extend(a1, a2);
    ck_assert_int_eq(extend_result, 0);

    intarray_free(a2);

    ck_assert_uint_eq(priv1->len, 1);
    ck_assert_uint_ge(priv1->capacity, priv1->len);
    ck_assert_int_eq(a1->items[0], value);
}
END_TEST


/*
 * Test extending a tsarray (of 1 item) with itself.
 */
START_TEST(test_extend_self_one)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int value = 33;
    int extend_result;

    append_seq_checked(a1, value, value+1);

    extend_result = intarray_extend(a1, a1);
    ck_assert_int_eq(extend_result, 0);

    ck_assert_uint_eq(priv->len, 2);
    ck_assert_uint_ge(priv->capacity, priv->len);
    ck_assert_int_eq(a1->items[0], value);
    ck_assert_int_eq(a1->items[1], value);
}
END_TEST


/*
 * Test extending a large tsarray with itself.
 *
 * This forces resizing of the tsarray, to test memory handling.
 */
START_TEST(test_extend_self_large)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int stop = 1024;
    int extend_result;
    int i;

    append_seq_checked(a1, 0, stop);

    extend_result = intarray_extend(a1, a1);
    ck_assert_int_eq(extend_result, 0);

    ck_assert_uint_eq(priv->len, 2*stop);
    ck_assert_uint_ge(priv->capacity, priv->len);
    for (i=0; i<stop; i++)
        ck_assert_int_eq(a1->items[i], i);
    for (i=0; i<stop; i++)
        ck_assert_int_eq(a1->items[stop+i], i);
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc_ops;

    s = suite_create("tsarray_extend");

    tc_ops = tcase_with_a1_create("extend");

    tcase_add_test(tc_ops, test_extend);
    tcase_add_test(tc_ops, test_extend_with_empty);
    tcase_add_test(tc_ops, test_extend_empty);
    tcase_add_test(tc_ops, test_extend_self_one);
    tcase_add_test(tc_ops, test_extend_self_large);

    suite_add_tcase(s, tc_ops);

    return s;
}


int main(void)
{
    Suite *s = tsarray_suite();
    int number_failed = run_tests(s);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
