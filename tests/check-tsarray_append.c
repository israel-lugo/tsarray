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
 * Test appending an element to an empty tsarray.
 */
START_TEST(test_append_one)
{
    append_seq_checked(a1, 10, 11);
}
END_TEST


/*
 * Test appending many items.
 *
 * Appends enough items to force array resizing.
 */
START_TEST(test_append_many)
{
    const int start = -1010;
    const int stop = 32010;
    const size_t expected_len = stop-start;
    int i;

    append_seq_checked(a1, start, stop);

    for (i=0; i<expected_len; i++)
    {
        const int value = i + start;
        ck_assert_int_eq(a1->items[i], value);
    }
}
END_TEST


/*
 * Test that tsarray_append detects overflow.
 */
START_TEST(test_append_overflow)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    const size_t old_capacity = priv->capacity;
    const size_t old_len = priv->len;
    int *const old_items = a1->items;
    int i = 5;
    int append_result;

    /* we cheat by messing with the internal structure, to avoid having to
     * actually append millions of objects */
    priv->len = priv->capacity = SIZE_MAX;

    append_result = intarray_append(a1, &i);
    ck_assert_int_eq(append_result, TSARRAY_EOVERFLOW);

    /* make sure append didn't change anything */
    ck_assert_uint_eq(priv->capacity, SIZE_MAX);
    ck_assert_uint_eq(priv->len, SIZE_MAX);
    ck_assert_ptr_eq(a1->items, old_items);

    /* undo the cheating, destructor may need the real values */
    priv->capacity = old_capacity;
    priv->len = old_len;
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc_ops;

    s = suite_create("tsarray");

    tc_ops = tcase_with_a1_create("operations");

    tcase_add_test(tc_ops, test_append_one);
    tcase_add_test(tc_ops, test_append_many);
    tcase_add_test(tc_ops, test_append_overflow);

    suite_add_tcase(s, tc_ops);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = tsarray_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=75 : */
