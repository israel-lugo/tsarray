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
 * check-static.c - check static functions
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "tsarray.c"

#include <stdlib.h>
#include <check.h>

/* get SIZE_MAX */
#include <stdint.h>

#include "common.h"


#define MAX_INDEX (min(SIZE_MAX, (unsigned long)LONG_MAX))


START_TEST(test_new_of_len)
{
    struct _tsarray_priv *a = _tsarray_new_of_len(sizeof(int), 1);

    ck_assert_ptr_ne(a, NULL);
    ck_assert_uint_eq(tsarray_len(&a->pub), 1);
    ck_assert_uint_ge(a->capacity, 1);
    tsarray_free(&a->pub);
}
END_TEST


static void check_new_capacity(size_t obj_size, unsigned long old_capacity,
        unsigned long new_len)
{
    unsigned long new_cap = calc_new_capacity(obj_size, old_capacity, new_len);

    ck_assert_uint_le(new_cap, SIZE_MAX);
    ck_assert(can_size_mult(new_cap, obj_size));
    ck_assert_uint_ge(new_cap, new_len);
}


static void check_new_capacity_with_hint(size_t obj_size,
        unsigned long old_capacity, unsigned long new_len,
        unsigned long len_hint)
{
    unsigned long new_cap = calc_new_capacity_with_hint(obj_size,
            old_capacity, new_len, len_hint);

    ck_assert_uint_le(new_cap, SIZE_MAX);
    ck_assert(can_size_mult(new_cap, obj_size));
    ck_assert_uint_ge(new_cap, new_len);
}


START_TEST(test_calc_new_capacity_incr)
{
    check_new_capacity(sizeof(int), 0, 0);
    check_new_capacity(sizeof(int), 0, 1);
    check_new_capacity(sizeof(int), 0, 1000);
    check_new_capacity(sizeof(int), 1, 1);
    check_new_capacity(sizeof(int), 1, 2);
    check_new_capacity(sizeof(int), 1, 1000);
    check_new_capacity(sizeof(int), 1000, 2000);
    check_new_capacity(1, 1000, 2000);
    check_new_capacity(1000, 32, 60);
    check_new_capacity(MAX_INDEX/128, 4, 128);
}
END_TEST


START_TEST(test_calc_new_capacity_decr)
{
    check_new_capacity(sizeof(int), 2, 1);
    check_new_capacity(sizeof(int), 1, 0);
    check_new_capacity(sizeof(int), 1000, 0);
    check_new_capacity(sizeof(int), 2000, 1000);
    check_new_capacity(1, 2000, 1000);
    check_new_capacity(1000, 60, 32);
    check_new_capacity(MAX_INDEX/128, 128, 4);
}
END_TEST


START_TEST(test_calc_new_capacity_hysteresis)
{
    unsigned long old_cap;
    unsigned long new_cap;

    old_cap = 30000;
    new_cap = calc_new_capacity(2, old_cap, old_cap-1);
    ck_assert_uint_eq(new_cap, old_cap);

    old_cap = MAX_INDEX/sizeof(int);
    new_cap = calc_new_capacity(sizeof(int), old_cap, old_cap-1);
    ck_assert_uint_eq(new_cap, old_cap);
}
END_TEST


START_TEST(test_calc_new_capacity_hint_incr)
{
    check_new_capacity_with_hint(sizeof(int), 0, 0, 0);
    check_new_capacity_with_hint(sizeof(int), 0, 0, 1);
    check_new_capacity_with_hint(sizeof(int), 0, 1, 0);
    check_new_capacity_with_hint(sizeof(int), 0, 100, 0);
    check_new_capacity_with_hint(sizeof(int), 0, 0, 100);
    check_new_capacity_with_hint(sizeof(int), 0, 1, 1);
    check_new_capacity_with_hint(sizeof(int), 0, 1, 100);
    check_new_capacity_with_hint(sizeof(int), 0, 1000, 100);
    check_new_capacity_with_hint(sizeof(int), 0, 1000, 2000);
    check_new_capacity_with_hint(sizeof(int), 1, 1, 1);
    check_new_capacity_with_hint(sizeof(int), 1, 2, 10);
    check_new_capacity_with_hint(sizeof(int), 1, 1000, 1000);
    check_new_capacity_with_hint(sizeof(int), 1000, 2000, 3003);
    check_new_capacity_with_hint(1, 1000, 2000, 2019);
    check_new_capacity_with_hint(1, 1000, 2000, MAX_INDEX);
    check_new_capacity_with_hint(1000, 32, 60, 57);
    check_new_capacity_with_hint(MAX_INDEX/128, 4, 128, 2);
    check_new_capacity_with_hint(MAX_INDEX/128, 4, 128, 128);
}
END_TEST


START_TEST(test_calc_new_capacity_hint_decr)
{
    check_new_capacity_with_hint(sizeof(int), 2, 1, 3);
    check_new_capacity_with_hint(sizeof(int), 1, 0, 2);
    check_new_capacity_with_hint(sizeof(int), 1, 0, 0);
    check_new_capacity_with_hint(sizeof(int), 1, 0, 10000);
    check_new_capacity_with_hint(sizeof(int), 1000, 0, 1000);
    check_new_capacity_with_hint(sizeof(int), 2000, 1000, 10000);
    check_new_capacity_with_hint(1, 2000, 1000, 1011);
    check_new_capacity_with_hint(1000, 60, 32, 57);
    check_new_capacity_with_hint(MAX_INDEX/128, 128, 4, 16);
    check_new_capacity_with_hint(MAX_INDEX/128, 128, 4, 128);
}
END_TEST


START_TEST(test_calc_new_capacity_hint_delta)
{
    unsigned long old_cap;
    unsigned long new_cap;

    old_cap = 30000;
    new_cap = calc_new_capacity_with_hint(2, old_cap, old_cap-100, old_cap);
    ck_assert_uint_eq(new_cap, old_cap);

    old_cap = MAX_INDEX/sizeof(int);
    new_cap = calc_new_capacity_with_hint(sizeof(int), old_cap, old_cap-1,
                                          old_cap);
    ck_assert_uint_eq(new_cap, old_cap);

    new_cap = calc_new_capacity_with_hint(sizeof(int), 0, 1, 1000);
    ck_assert_uint_ge(new_cap, 100);

    new_cap = calc_new_capacity_with_hint(sizeof(int), 10000, 44, 1000);
    ck_assert_uint_ge(new_cap, 100);
    ck_assert_uint_le(new_cap, 800);
}
END_TEST


/* TODO: Test tsarray_resize() and any other important static functions. */


Suite *static_suite(void)
{
    Suite *s;
    TCase *tc_static;

    s = suite_create("tsarray");

    tc_static = tcase_create("static");

    tcase_add_test(tc_static, test_new_of_len);
    tcase_add_test(tc_static, test_calc_new_capacity_incr);
    tcase_add_test(tc_static, test_calc_new_capacity_decr);
    tcase_add_test(tc_static, test_calc_new_capacity_hysteresis);
    tcase_add_test(tc_static, test_calc_new_capacity_hint_incr);
    tcase_add_test(tc_static, test_calc_new_capacity_hint_decr);
    tcase_add_test(tc_static, test_calc_new_capacity_hint_delta);
    suite_add_tcase(s, tc_static);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = static_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
