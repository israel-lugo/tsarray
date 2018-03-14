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



static int intcmp(const int *a, const int *b, void *arg)
{
    (void)arg;
    return *a > *b ? 1 : (*a == *b ? 0 : -1);
}


/*
 * Test finding min and max for an empty array.
 */
START_TEST(test_empty)
{
    int *minval = intarray_min(a1, intcmp);
    ck_assert_ptr_eq(minval, NULL);

    int *maxval = intarray_max(a1, intcmp);
    ck_assert_ptr_eq(maxval, NULL);
}
END_TEST


/*
 * Test finding min and max of an array with one item.
 */
START_TEST(test_single)
{
    const int start = 47;
    int *minval;
    int *maxval;

    append_seq_checked(a1, start, start+1);

    minval = intarray_min(a1, intcmp);
    ck_assert_ptr_ne(minval, NULL);
    ck_assert_ptr_eq(minval, &a1->items[0]);
    ck_assert_int_eq(*minval, start);

    maxval = intarray_max(a1, intcmp);
    ck_assert_ptr_ne(maxval, NULL);
    ck_assert_ptr_eq(maxval, &a1->items[0]);
    ck_assert_int_eq(*maxval, start);
}
END_TEST


/*
 * Test finding min and max of an array with two items.
 */
START_TEST(test_two_items)
{
    const int start = 0;
    int *minval;
    int *maxval;

    append_seq_checked(a1, start, start+2);

    minval = intarray_min(a1, intcmp);
    ck_assert_ptr_ne(minval, NULL);
    ck_assert_ptr_eq(minval, &a1->items[0]);
    ck_assert_int_eq(*minval, start);

    maxval = intarray_max(a1, intcmp);
    ck_assert_ptr_ne(maxval, NULL);
    ck_assert_ptr_eq(maxval, &a1->items[1]);
    ck_assert_int_eq(*maxval, start+1);
}
END_TEST


/*
 * Test finding min and max of an array with duplicate items.
 */
START_TEST(test_duplicate)
{
    int x = 47;
    int *minval;
    int *maxval;

    intarray_append(a1, &x);
    intarray_append(a1, &x);
    intarray_append(a1, &x);

    minval = intarray_min(a1, intcmp);
    ck_assert_ptr_ne(minval, NULL);
    ck_assert_ptr_eq(minval, &a1->items[0]);
    ck_assert_int_eq(*minval, x);

    maxval = intarray_max(a1, intcmp);
    ck_assert_ptr_ne(maxval, NULL);
    ck_assert_ptr_eq(maxval, &a1->items[0]);
    ck_assert_int_eq(*maxval, x);
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("tsarray_minmax");

    tc = tcase_with_a1_create("minmax");

    tcase_add_test(tc, test_empty);
    tcase_add_test(tc, test_single);
    tcase_add_test(tc, test_two_items);
    tcase_add_test(tc, test_duplicate);

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
