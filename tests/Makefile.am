
# programs built only with "make check"; don't include in "make all"
check_PROGRAMS = check-internal check-tsarray check-tsarray_append check-tsarray_remove check-tsarray_extend check-tsarray_slice test-array test-sparse

# run these programs as tests when doing "make check"
TESTS = $(check_PROGRAMS)

AM_CFLAGS = -I$(top_srcdir)/src

libs_path = $(top_builddir)/src/.libs

tsarray_common_sources = setupcheck.c setupcheck.h $(top_builddir)/src/tsarray.h
tsarray_common_cflags = $(AM_CFLAGS) @CHECK_CFLAGS@
tsarray_common_ldadd = $(libs_path)/libtsarray.la @CHECK_LIBS@

check_internal_SOURCES = check-internal.c $(top_builddir)/src/common.h
check_internal_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
check_internal_LDADD = @CHECK_LIBS@

check_tsarray_SOURCES = check-tsarray.c $(tsarray_common_sources)
check_tsarray_CFLAGS = $(tsarray_common_cflags)
check_tsarray_LDADD = $(tsarray_common_ldadd)

check_tsarray_append_SOURCES = check-tsarray_append.c $(tsarray_common_sources)
check_tsarray_append_CFLAGS = $(tsarray_common_cflags)
check_tsarray_append_LDADD = $(tsarray_common_ldadd)

check_tsarray_remove_SOURCES = check-tsarray_remove.c $(tsarray_common_sources)
check_tsarray_remove_CFLAGS = $(tsarray_common_cflags)
check_tsarray_remove_LDADD = $(tsarray_common_ldadd)

check_tsarray_extend_SOURCES = check-tsarray_extend.c $(tsarray_common_sources)
check_tsarray_extend_CFLAGS = $(tsarray_common_cflags)
check_tsarray_extend_LDADD = $(tsarray_common_ldadd)

check_tsarray_slice_SOURCES = check-tsarray_slice.c $(tsarray_common_sources)
check_tsarray_slice_CFLAGS = $(tsarray_common_cflags)
check_tsarray_slice_LDADD = $(tsarray_common_ldadd)

test_array_LDADD = $(libs_path)/libtsarray.la
test_array_SOURCES = test-array.c $(top_builddir)/src/tsarray.h
test_sparse_SOURCES = test-sparse.c $(top_builddir)/src/tssparse.h
test_sparse_LDADD = $(libs_path)/libtssparse.la

