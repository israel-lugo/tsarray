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

#ifndef _SETUPCHECK_H
#define _SETUPCHECK_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <check.h>

#include <tsarray.h>


/*
 * Copied here from src/tsarray.c. This is internal use only. Must keep in
 * sync with the definition in src/tsarray.c.
 */
struct _tsarray_priv {
    struct _tsarray_pub pub;
    size_t obj_size;
    size_t capacity;
    size_t len;
};

TSARRAY_TYPEDEF(intarray, int);

extern intarray *a1;

void append_seq_checked(intarray *a, int start, int stop);
TCase *tcase_with_a1_create(const char *name);
int run_tests(Suite *s);


#endif  /* _SETUPCHECK_H */


/* vim: set expandtab smarttab shiftwidth=4 softtabstop=4 tw=78 : */
