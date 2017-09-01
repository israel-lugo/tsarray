
# programs built only with "make check"; don't include in "make all"
check_PROGRAMS = check-tsarray test-array test-sparse

# run these programs as tests when doing "make check"
TESTS = $(check_PROGRAMS)

AM_CFLAGS = -I$(top_srcdir)/src

libs_path = $(top_builddir)/src/.libs

check_tsarray_SOURCES = check-tsarray.c $(top_builddir)/src/tsarray.h
check_tsarray_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
check_tsarray_LDADD = $(libs_path)/libtsarray.la @CHECK_LIBS@

test_array_LDADD = $(libs_path)/libtsarray.la
test_array_SOURCES = test-array.c $(top_builddir)/src/tsarray.h
test_sparse_SOURCES = test-sparse.c $(top_builddir)/src/tssparse.h
test_sparse_LDADD = $(libs_path)/libtssparse.la

