

common_headers = common.h compiler.h

lib_LTLIBRARIES = libtsarray.la libtssparse.la
libtsarray_la_SOURCES = tsarray.c tsarray.h $(common_headers)
libtssparse_la_SOURCES = tssparse.c tssparse.h $(common_headers)

include_HEADERS = tsarray.h tssparse.h

bin_PROGRAMS = test-array test-sparse test2-sparse
test_array_LDADD = libtsarray.la
test_sparse_LDADD = libtssparse.la
test2_sparse_LDADD = libtssparse.la

