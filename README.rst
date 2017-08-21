tsarray
=======

|License|

Generic type-safe dynamic array library for C.

tsarray is a C implementation of a generic dynamic array, that is type safe at
compile-time. This means several things:

generic
  tsarrays can be used to store objects of any given type.

type-safe
  A given tsarray will only accept objects of its specified type, i.e.: a
  tsarray of ``int`` will only accept ``int``. This is enforced at
  compile-time.

dynamic
  tsarrays are resizable; they will automatically grow or shrink as new items
  are added or removed;


Concept
-------

The concept behind tsarray is simple: to store objects of a certain type, one
must first declare a specific "subtype" of tsarray, that accepts that type of
object (and only that type).

This mechanism of defining specific "subtypes" of tsarray can be thought of as
akin to subclassing, from an object-oriented programming (OOP) perspective. In
an OOP perspective, tsarray would be an abstract base class, which one must
subclass to create specific arrays that know how to handle a certain type.

Another similar concept would be C++'s notion of templates. A tsarray basically
implements parametrized polymorphism in C.


Usage
-----

The ``TSARRAY_TYPEDEF(arraytype, objtype)`` macro will define (typedef) a new
type ``arraytype`` and all the type-specific manipulator functions. These
functions are named with the prefix ``arraytype_*``, e.g.
``intarray_append()``, etc.  They work with objects of the specified
``objtype``.

For example, the code ``TSARRAY_TYPEDEF(intarray, int);`` will declare a new
type ``intarray``, which is a tsarray that holds objects of type ``int``. It
will create functions like ``intarray_append(intarray *array, int *obj)``,
which appends the integer pointed-to by ``obj`` to the end of ``array``.


Example
-------

.. code:: c

    #include <stdio.h>
    #include "tsarray.h"

    /* declare a new typedef called intarray, for a tsarray of int */
    TSARRAY_TYPEDEF(intarray, int);

    void f(int a, int b) {
        intarray a1 = TSARRAY_INITIALIZER;

        intarray_append(&a1, &a);
        intarray_append(&a1, &b);

        printf("a1.len: %lu\n", a1.len);
        printf("a1[0]: %d\n", a1.items[0]);
        printf("a1[1]: %d\n", a1.items[1]);

        puts("Removing a[0]...");
        intarray_remove(&a1, 0);

        printf("a1.len: %lu\n", a1.len);
        printf("a1[0]: %d\n", a1.items[0]);

        intarray_free(&a1);
    }

Given the above code, calling ``f(31, 42)`` would produce the following output::

  a1.len: 2
  a1[0]: 31
  a1[1]: 42
  Removing a[0]...
  a1.len: 1
  a1[0]: 42


Contact
-------

tsarray is developed by Israel G. Lugo <israel.lugo@lugosys.com>. Main
repository for cloning, submitting issues and/or forking is at
https://github.com/israel-lugo/tsarray


License
-------

Copyright (C) 2012, 2015, 2016, 2017 Israel G. Lugo <israel.lugo@lugosys.com>

tsarray is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tsarray is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tsarray.  If not, see <http://www.gnu.org/licenses/>.


.. |License| image:: https://img.shields.io/badge/license-GPLv3+-blue.svg?maxAge=2592000
   :target: COPYING
