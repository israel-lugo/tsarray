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


TSARRAY_TYPEDEF(intarray, int);

intarray *a1 = NULL;


/*
 * Create a new intarray and make a1 point to it.
 *
 * To be used as the setup for a checked test fixture.
 */
void new_array(void)
{
    a1 = intarray_new();
    ck_assert_ptr_ne(a1, NULL);
}


/*
 * Free the a1 intarray.
 *
 * To be used as the teardown for a checked test fixture.
 */
void del_array(void)
{
    ck_assert_int_eq((a1->items == NULL), (a1->_priv.capacity == 0));
    intarray_free(a1);
}


/*
 * Make sure abstract tsarray is the same size as a type-specific "subclass".
 *
 * We rely on this for the constructor, and elsewhere.
 *
 * This is tested in a test case of its own, without the test fixture that
 * initializes an empty tsarray. The whole point is to catch any potential
 * problems before the constructor hits them.
 */
START_TEST(test_tsarray_size)
{
    /* tsarray_new() assumes this much */
    ck_assert_uint_eq(sizeof(intarray), sizeof(struct _tsarray_abs));
}
END_TEST


/*
 * Test creating and destroying a tsarray.
 */
START_TEST(test_create_and_free)
{
    ck_assert_uint_eq(a1->len, 0);
    ck_assert_uint_ge(a1->_priv.capacity, 0);
}
END_TEST


/*
 * Test appending an element to an empty tsarray.
 */
START_TEST(test_append)
{
    int i = 5;
    int append_result;

    append_result = intarray_append(a1, &i);
    ck_assert_int_eq(append_result, 0);

    ck_assert_uint_eq(a1->len, 1);
    ck_assert_int_eq(a1->items[0], i);
}
END_TEST


/*
 * Test that tsarray_append detects overflow.
 */
START_TEST(test_append_overflow)
{
    const size_t old_capacity = a1->_priv.capacity;
    const size_t old_len = a1->len;
    int i = 5;
    int append_result;

    /* we cheat by messing with the internal structure, to avoid having to
     * actually append millions of objects */
    a1->len = a1->_priv.capacity = SIZE_MAX;

    append_result = intarray_append(a1, &i);
    ck_assert_int_eq(append_result, TSARRAY_EOVERFLOW);

    /* undo the cheating, destructor may need the real values */
    a1->_priv.capacity = old_capacity;
    a1->len = old_len;
}
END_TEST


/*
 * Test removing an element from an array.
 */
START_TEST(test_remove)
{
    int i = 5;
    int append_result, remove_result;

    append_result = intarray_append(a1, &i);
    ck_assert_int_eq(append_result, 0);

    remove_result = intarray_remove(a1, 0);
    ck_assert_int_eq(remove_result, 0);

    ck_assert_uint_eq(a1->len, 0);
}
END_TEST


Suite *foo_suite(void)
{
    Suite *s;
    TCase *tc_memsizes;
    TCase *tc_ops;

    s = suite_create("tsarray");

    tc_memsizes = tcase_create("memsizes");
    tc_ops = tcase_create("operations");

    tcase_add_test(tc_memsizes, test_tsarray_size);

    tcase_add_checked_fixture(tc_ops, new_array, del_array);
    tcase_add_test(tc_ops, test_create_and_free);
    tcase_add_test(tc_ops, test_append);
    tcase_add_test(tc_ops, test_append_overflow);
    tcase_add_test(tc_ops, test_remove);

    suite_add_tcase(s, tc_memsizes);
    suite_add_tcase(s, tc_ops);

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
