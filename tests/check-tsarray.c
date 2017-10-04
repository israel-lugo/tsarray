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
 * Test appending an element to an empty tsarray.
 */
START_TEST(test_append_one)
{
    append_seq_checked(a1, 10, 11);
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
    tcase_add_test(tc_ops, test_len_empty);
    tcase_add_test(tc_ops, test_from_array);
    tcase_add_test(tc_ops, test_from_array_empty);
    tcase_add_test(tc_ops, test_append_one);
    tcase_add_test(tc_ops, test_len_one);
    tcase_add_test(tc_ops, test_append_many);
    tcase_add_test(tc_ops, test_append_overflow);
    tcase_add_test(tc_ops, test_copy);
    tcase_add_test(tc_ops, test_copy_empty);
    tcase_add_test(tc_ops, test_extend);
    tcase_add_test(tc_ops, test_extend_with_empty);
    tcase_add_test(tc_ops, test_extend_empty);
    tcase_add_test(tc_ops, test_extend_self_one);
    tcase_add_test(tc_ops, test_extend_self_large);
    tcase_add_test(tc_ops, test_set_then_move);

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
