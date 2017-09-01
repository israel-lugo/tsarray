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

#include <tsarray.h>


TSARRAY_TYPEDEF(intarray, int);



START_TEST(test_create_and_free)
{
    intarray a1 = TSARRAY_INITIALIZER;

    ck_assert_uint_eq(a1.len, 0);
    ck_assert_uint_ge(a1._priv.capacity, 0);
    ck_assert_int_eq((a1.items == NULL), (a1._priv.capacity == 0));

    intarray_free(&a1);
}
END_TEST

START_TEST(test_append)
{
    intarray a1 = TSARRAY_INITIALIZER;
    int i = 5;
    int append_result;

    append_result = intarray_append(&a1, &i);
    ck_assert_int_eq(append_result, 0);

    ck_assert_uint_eq(a1.len, 1);
    ck_assert_int_eq(a1.items[0], i);

    intarray_free(&a1);
}
END_TEST

START_TEST(test_remove)
{
    intarray a1 = TSARRAY_INITIALIZER;
    int i = 5;
    int append_result, remove_result;

    append_result = intarray_append(&a1, &i);
    ck_assert_int_eq(append_result, 0);

    remove_result = intarray_remove(&a1, 0);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(a1.len, 0);

    intarray_free(&a1);
}
END_TEST


Suite *foo_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("tsarray");

    tc = tcase_create("core");

    tcase_add_test(tc, test_create_and_free);
    tcase_add_test(tc, test_append);
    tcase_add_test(tc, test_remove);
    suite_add_tcase(s, tc);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = foo_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
