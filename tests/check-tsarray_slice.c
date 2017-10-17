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
 * Helper function to test slicing N items past the end of a tsarray.
 *
 * Receives the size of the desired source tsarray, the desired start index
 * for the slicing, and the number of items past the end of the source
 * tsarray to include in the slice.
 *
 * slice_start MUST be <= src_len.
 *
 * Checks that a new tsarray is created, with the expected length and
 * contents (up to the end of the source tsarray, but not beyond). Will
 * fail the test if any of the internal checks fail.
 */
static void check_slice_past_n(size_t src_len, size_t slice_start, size_t past)
{
    const int start = -100;
    const int stop = start+src_len;
    const size_t expected_slice_len = src_len-slice_start;
    intarray *aslice;
    int i;

    append_seq_checked(a1, start, stop);

    aslice = intarray_slice(a1, slice_start, intarray_len(a1)+past, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), expected_slice_len);
    for (i=0; i<expected_slice_len; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[i+slice_start]);

    intarray_free(aslice);
}


/*
 * Test slicing a single item from a tsarray.
 */
START_TEST(test_slice_one)
{
    const int stop = 10;
    const size_t sliceidx = 4;
    intarray *aslice;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, sliceidx, sliceidx+1, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    /* len(aslice) == 1 and a1[sliceidx] == aslice[0] */
    ck_assert_uint_eq(intarray_len(aslice), 1);
    ck_assert_int_eq(aslice->items[0], a1->items[sliceidx]);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing some items from a tsarray.
 */
START_TEST(test_slice_some)
{
    const int stop = 10;
    const size_t slice_start = 4;
    const size_t slice_stop = 8;
    const size_t expected_slice_len = slice_stop-slice_start;
    intarray *aslice;
    int i;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), expected_slice_len);

    for (i=0; i<expected_slice_len; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[i+slice_start]);
}
END_TEST


/*
 * Test slicing some items from a tsarray in reverse.
 *
 * Given:
 *      a1 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
 * Checks that:
 *      a1[8:4:-1] == [8, 7, 6, 5]
 */
START_TEST(test_slice_some_reverse)
{
    const int stop = 10;
    const size_t slice_start = 8;
    const size_t slice_stop = 4;
    const size_t expected_slice_len = slice_start-slice_stop;
    intarray *aslice;
    int i;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, -1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), expected_slice_len);

    for (i=0; i<expected_slice_len; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[slice_start-i]);
}
END_TEST


/*
 * Test slicing some items from a tsarray, with a step.
 */
START_TEST(test_slice_some_step)
{
    const int stop = 100;
    const size_t slice_start = 4;
    const size_t slice_stop = 50;
    const int slice_step = 3;
    const size_t expected_slice_len = (slice_stop - slice_start)/slice_step + 1;
    intarray *aslice;
    int i;

    /* make sure test parameters are correctly in bounds */
    ck_assert_int_lt(slice_start + (expected_slice_len-1)*slice_step, stop);

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, slice_step);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), expected_slice_len);

    for (i=0; i<expected_slice_len; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[slice_start + i*slice_step]);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing some items from a tsarray, with a negative step.
 */
START_TEST(test_slice_some_step_reverse)
{
    const int stop = 100;
    const size_t slice_start = 50;
    const size_t slice_stop = 4;
    const int slice_step = -3;
    const size_t expected_slice_len = (slice_start - slice_stop)/(-slice_step) + 1;
    intarray *aslice;
    int i;

    /* make sure test parameters are correctly in bounds */
    ck_assert_int_lt(slice_stop + 1 + (expected_slice_len-1)*(-slice_step), stop);

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, slice_step);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), expected_slice_len);

    for (i=0; i<expected_slice_len; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[slice_start + i*slice_step]);

    intarray_free(aslice);
}
END_TEST


/*
 * Test creating a slice with a step larger than the slice range.
 *
 * Makes sure the slice contains the first item and nothing else.
 */
START_TEST(test_slice_step_too_large)
{
    const int stop = 100;
    const size_t slice_start = 14;
    const size_t slice_stop = 50;
    intarray *aslice;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, slice_stop-slice_start);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), 1);
    ck_assert_int_eq(aslice->items[0], a1->items[slice_start]);

    intarray_free(aslice);
}
END_TEST


/*
 * Test creating an empty slice from a non-empty tsarray.
 */
START_TEST(test_slice_none)
{
    const int stop = 10;
    const size_t sliceidx = 4;
    intarray *aslice;

    append_seq_checked(a1, 0, stop);

    /* aslice = a1[sliceidx:sliceidx] */
    aslice = intarray_slice(a1, sliceidx, sliceidx, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing with start > stop and positive step.
 *
 * Makes sure the slice is empty.
 */
START_TEST(test_slice_start_past_stop)
{
    const int stop = 10;
    const size_t slice_start = 7;
    const size_t slice_stop = 3;
    intarray *aslice;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing with start < stop and negative step.
 *
 * Makes sure the slice is empty.
 */
START_TEST(test_slice_back_start_before_stop)
{
    const int stop = 10;
    const size_t slice_start = 3;
    const size_t slice_stop = 7;
    intarray *aslice;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, slice_start, slice_stop, -1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing from an empty tsarray.
 *
 * Makes sure the resulting slice is empty.
 */
START_TEST(test_slice_from_empty)
{
    intarray *aslice;

    /* a1 = []; aslice = a1[0:1:1] */
    aslice = intarray_slice(a1, 0, 1, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing nothing from an empty tsarray.
 *
 * Slicing nothing from an empty tsarray should succeed, as we don't need
 * any content.
 */
START_TEST(test_slice_none_from_empty)
{
    intarray *aslice;

    /* aslice = a1[0:0] */
    aslice = intarray_slice(a1, 0, 0, 1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);
    /* aslice is empty */
    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);

    /* aslice = a1[0:1:3], must work since step==3 > stop-start==1 */
    aslice = intarray_slice(a1, 0, 1, 3);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);
    /* aslice is empty */
    ck_assert_uint_eq(intarray_len(aslice), 0);

    intarray_free(aslice);
}
END_TEST


/*
 * Test slicing all items from a tsarray.
 */
START_TEST(test_slice_all)
{
    check_slice_past_n(100, 0, 0);
}
END_TEST


/*
 * Test slicing all items from a tsarray, including one past the end.
 */
START_TEST(test_slice_all_past_one)
{
    check_slice_past_n(100, 0, 1);
}
END_TEST


/*
 * Test slicing many past the end of a tsarray.
 */
START_TEST(test_slice_past_many)
{
    check_slice_past_n(100, 14, 10000);
}
END_TEST


/*
 * Test slicing all items from a tsarray with a negative step.
 */
START_TEST(test_slice_all_reverse)
{
    const int stop = 100;
    intarray *aslice;
    int i;

    append_seq_checked(a1, 0, stop);

    aslice = intarray_slice(a1, stop, 0, -1);

    /* aslice was successfully created and is not the same as a1 */
    ck_assert_ptr_ne(aslice, NULL);
    ck_assert_ptr_ne(aslice, a1);

    ck_assert_uint_eq(intarray_len(aslice), intarray_len(a1));

    for (i=0; i<stop; i++)
        ck_assert_int_eq(aslice->items[i], a1->items[stop-i-1]);

    intarray_free(aslice);
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("tsarray_slice");

    tc = tcase_with_a1_create("slice");

    tcase_add_test(tc, test_slice_one);
    tcase_add_test(tc, test_slice_some);
    tcase_add_test(tc, test_slice_some_reverse);
    tcase_add_test(tc, test_slice_some_step);
    tcase_add_test(tc, test_slice_some_step_reverse);
    tcase_add_test(tc, test_slice_step_too_large);
    tcase_add_test(tc, test_slice_none);
    tcase_add_test(tc, test_slice_start_past_stop);
    tcase_add_test(tc, test_slice_back_start_before_stop);
    tcase_add_test(tc, test_slice_from_empty);
    tcase_add_test(tc, test_slice_none_from_empty);
    tcase_add_test(tc, test_slice_all);
    tcase_add_test(tc, test_slice_all_past_one);
    tcase_add_test(tc, test_slice_past_many);
    tcase_add_test(tc, test_slice_all_reverse);

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
