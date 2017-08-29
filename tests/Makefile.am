
# programs built only with "make check"; don't include in "make all"
check_PROGRAMS = test-array test-sparse test2-sparse

# run these programs as tests when doing "make check"
TESTS = $(check_PROGRAMS)

AM_CFLAGS = -I$(top_srcdir)/src

libs_path = $(top_builddir)/src/.libs

test_array_LDADD = $(libs_path)/libtsarray.la
test_sparse_LDADD = $(libs_path)/libtssparse.la
test2_sparse_LDADD = $(libs_path)/libtssparse.la

