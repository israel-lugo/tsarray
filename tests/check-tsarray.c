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
 * Test creating and destroying a tsarray.
 */
START_TEST(test_create_and_free)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a1;
    ck_assert_uint_eq(priv->len, 0);
    ck_assert_uint_ge(priv->capacity, 0);
}
END_TEST


/*
 * Test getting the length of an empty tsarray.
 */
START_TEST(test_len_empty)
{
    ck_assert_uint_eq(intarray_len(a1), 0);
}
END_TEST


/*
 * Test getting the length of a tsarray of 1 element.
 */
START_TEST(test_len_one)
{
    append_seq_checked(a1, 10, 11);
    ck_assert_uint_eq(intarray_len(a1), 1);
}
END_TEST


/*
 * Test creating a tsarray from a normal C array.
 */
START_TEST(test_from_array)
{
    static const int src[] = {
        15, 66, 98, -7, 1, INT_MIN, -9, -45, 3, 0, -1, 15, INT_MAX
    };
    const size_t srclen = sizeof(src) / sizeof(src[0]);
    intarray *b = intarray_from_array(src, srclen);
    struct _tsarray_priv *privb = (struct _tsarray_priv *)b;
    int i;

    ck_assert_ptr_ne(b, NULL);
    ck_assert_ptr_ne(b->items, src);
    ck_assert_uint_eq(privb->len, srclen);
    ck_assert_uint_ge(privb->capacity, privb->len);

    for (i=0; i<srclen; i++)
        ck_assert_int_eq(b->items[i], src[i]);

    intarray_free(b);
}
END_TEST


/*
 * Test creating a tsarray from an empty C array.
 *
 * Makes sure that tsarray_from_array(x, 0) doesn't read from x. In
 * particular, x can be NULL.
 */
START_TEST(test_from_array_empty)
{
    intarray *b = intarray_from_array(NULL, 0);
    struct _tsarray_priv *privb = (struct _tsarray_priv *)b;

    ck_assert_ptr_ne(b, NULL);
    ck_assert_uint_eq(privb->len, 0);

    intarray_free(b);
}
END_TEST


/*
 * Test creating a copy of a tsarray.
 */
START_TEST(test_copy)
{
    struct _tsarray_priv *priv1 = (struct _tsarray_priv *)a1;
    struct _tsarray_priv *priv2;
    const int stop = 20;
    intarray *a2;
    int i;

    append_seq_checked(a1, 0, stop);

    a2 = intarray_copy(a1);
    priv2 = (struct _tsarray_priv *)a2;

    ck_assert_ptr_ne(a2, NULL);
    ck_assert_ptr_ne(a2, a1);
    ck_assert_uint_eq(priv2->len, priv1->len);
    ck_assert_uint_ge(priv2->capacity, priv2->len);

    for (i=0; i<stop; i++)
        ck_assert_int_eq(a2->items[i], a1->items[i]);

    intarray_free(a2);
}
END_TEST


/*
 * Test copying an empty tsarray.
 */
START_TEST(test_copy_empty)
{
    intarray *a2 = intarray_copy(a1);
    struct _tsarray_priv *priv2 = (struct _tsarray_priv *)a2;

    ck_assert_ptr_ne(a2, NULL);
    ck_assert_ptr_ne(a2, a1);
    ck_assert_uint_eq(priv2->len, 0);

    intarray_free(a2);
}
END_TEST


/*
 * Test setting items on a tsarray then removing something to the left.
 *
 * This test is an attempt to catch invalid instruction reordering -- where
 * the compiler might reorder the checks to happen before the remove.
 *
 * This could happen if there is a pointer aliasing issue on our API, which
 * causes the compiler to think that intarray_remove is not working on the
 * same object that is being accessed by the checks.
 */
START_TEST(test_set_then_move)
{
    static const int src[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    intarray *const b = intarray_from_array(src, sizeof(src) / sizeof(src[0]));
    int remove_result;

    b->items[7] = -7;
    b->items[8] = -8;

    remove_result = intarray_remove(b, 6);

    /* this check should always fail, since remove moves the items around */
    if (b->items[8] == -8)
    {   /* should never enter this; we've been reordered */
        b->items[9] = 99;
    }

    /* remove was successful */
    ck_assert_int_eq(remove_result, 0);

    /* value was moved to the left */
    ck_assert_int_ne(b->items[7], -7);
    ck_assert_int_ne(b->items[8], -8);
    ck_assert_int_eq(b->items[6], -7);
    ck_assert_int_eq(b->items[7], -8);

    /* did not enter impossible if */
    ck_assert_int_ne(b->items[9], 99);

    intarray_free(b);
}
END_TEST


Suite *tsarray_suite(void)
{
    Suite *s;
    TCase *tc_ops;

    s = suite_create("tsarray");

    tc_ops = tcase_with_a1_create("operations");

    tcase_add_test(tc_ops, test_create_and_free);
    tcase_add_test(tc_ops, test_from_array);
    tcase_add_test(tc_ops, test_from_array_empty);
    tcase_add_test(tc_ops, test_len_empty);
    tcase_add_test(tc_ops, test_len_one);
    tcase_add_test(tc_ops, test_copy);
    tcase_add_test(tc_ops, test_copy_empty);
    tcase_add_test(tc_ops, test_set_then_move);

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
