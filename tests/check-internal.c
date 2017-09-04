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
 * check-internal.c - check internal helper functions
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <check.h>

/* get SIZE_MAX */
#include <stdint.h>

#include "common.h"


START_TEST(test_can_int_add)
{
    ck_assert(can_int_add(0, 0));
    ck_assert(can_int_add(1, 1));
    ck_assert(can_int_add(1, -1));
    ck_assert(can_int_add(0, INT_MAX));
    ck_assert(can_int_add(INT_MAX, 0));
    ck_assert(can_int_add(INT_MAX, -1));
    ck_assert(can_int_add(INT_MIN, 1));
    ck_assert(can_int_add(INT_MIN, INT_MAX));
    ck_assert(can_int_add(INT_MAX/2, INT_MAX/2));
    ck_assert(!can_int_add(INT_MAX, 1));
    ck_assert(!can_int_add(1, INT_MAX));
    ck_assert(!can_int_add(INT_MIN, INT_MIN));
    ck_assert(!can_int_add(INT_MAX, INT_MAX));
    ck_assert(!can_int_add(INT_MAX, INT_MAX-1));
    ck_assert(!can_int_add(INT_MAX/2, INT_MAX/2 + 2));
}
END_TEST


START_TEST(test_can_size_add)
{
    ck_assert(can_size_add(0, 0));
    ck_assert(can_size_add(0, SIZE_MAX));
    ck_assert(can_size_add(SIZE_MAX, 0));
    ck_assert(can_size_add(1, 1));
    ck_assert(can_size_add(SIZE_MAX/2, SIZE_MAX/2));
    ck_assert(!can_size_add(SIZE_MAX, 1));
    ck_assert(!can_size_add(1, SIZE_MAX));
    ck_assert(!can_size_add(SIZE_MAX, SIZE_MAX));
    ck_assert(!can_size_add(SIZE_MAX, SIZE_MAX-1));
    ck_assert(!can_size_add(SIZE_MAX/2, SIZE_MAX/2 + 2));
}
END_TEST


START_TEST(test_can_size_mult)
{
    ck_assert(can_size_mult(0, 0));
    ck_assert(can_size_mult(1, 0));
    ck_assert(can_size_mult(0, 1));
    ck_assert(can_size_mult(1, 1));
    ck_assert(can_size_mult(SIZE_MAX, 0));
    ck_assert(can_size_mult(0, SIZE_MAX));
    ck_assert(can_size_mult(SIZE_MAX, 1));
    ck_assert(can_size_mult(SIZE_MAX/2, 2));
    ck_assert(!can_size_mult(SIZE_MAX, 2));
    ck_assert(!can_size_mult(SIZE_MAX, SIZE_MAX));
}
END_TEST


Suite *internal_suite(void)
{
    Suite *s;
    TCase *tc_overflow;

    s = suite_create("internal");

    tc_overflow = tcase_create("overflow");

    tcase_add_test(tc_overflow, test_can_int_add);
    tcase_add_test(tc_overflow, test_can_size_add);
    tcase_add_test(tc_overflow, test_can_size_mult);

    suite_add_tcase(s, tc_overflow);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = internal_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
