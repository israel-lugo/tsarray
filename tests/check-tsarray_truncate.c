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

#include <limits.h>

#include <tsarray.h>

#include "setupcheck.h"


#define ARG_PTR ((void *)0x1234)



/*
 * Test truncating an empty array.
 */
START_TEST(test_truncate_empty)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    int retval = intarray_truncate(a1, 0);
    ck_assert_int_eq(retval, 0);

    ck_assert_uint_eq(priv->len, 0);
}
END_TEST


/*
 * Test truncating an array of one item.
 */
START_TEST(test_truncate_one)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int value = 33;
    int retval;
   
    append_seq_checked(a1, value, value+1);

    retval = intarray_truncate(a1, 0);
    ck_assert_int_eq(retval, 0);

    ck_assert_uint_eq(priv->len, 0);
}
END_TEST


/*
 * Test truncating an array of one item to its same size.
 */
START_TEST(test_truncate_one_same)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int value = 33;
    int retval;
   
    append_seq_checked(a1, value, value+1);

    retval = intarray_truncate(a1, 1);
    ck_assert_int_eq(retval, 0);

    ck_assert_uint_eq(priv->len, 1);
    ck_assert_uint_ge(priv->capacity, priv->len);
    ck_assert_int_eq(a1->items[0], value);
}
END_TEST


/*
 * Test truncating an array of many items to one.
 */
START_TEST(test_truncate_many_to_one)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int start = 33;
    const int n = 1255;
    int retval;
    int i;
   
    append_seq_checked(a1, start, start+n);

    retval = intarray_truncate(a1, 1);
    ck_assert_int_eq(retval, 0);

    ck_assert_uint_eq(priv->len, 1);
    ck_assert_uint_ge(priv->capacity, priv->len);
    ck_assert_int_eq(a1->items[0], start);
}
END_TEST


/*
 * Test truncating an array of many items minus one.
 */
START_TEST(test_truncate_many_minus_one)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const int start = 33;
    const int n = 1255;
    int retval;
    int i;
   
    append_seq_checked(a1, start, start+n);

    retval = intarray_truncate(a1, n-1);
    ck_assert_int_eq(retval, 0);

    ck_assert_uint_eq(priv->len, (unsigned int)n-1);
    ck_assert_uint_ge(priv->capacity, priv->len);
    for (i=0; i<n-1; i++)
        ck_assert_int_eq(a1->items[i], start+i);
}
END_TEST




Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("tsarray_truncate");

    tc = tcase_with_a1_create("truncate");

    tcase_add_test(tc, test_truncate_empty);
    tcase_add_test(tc, test_truncate_one);
    tcase_add_test(tc, test_truncate_one_same);
    tcase_add_test(tc, test_truncate_many_to_one);
    tcase_add_test(tc, test_truncate_many_minus_one);

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
