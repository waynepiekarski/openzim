noinst_PROGRAMS = indexertest
bin_PROGRAMS = zimindexer
#include_HEADERS = zimindexer.h

AM_CPPFLAGS=-I$(top_builddir)/include

zimindexer_SOURCES = articleparser.cpp zimindexer.cpp
indexertest_SOURCES = articleparser.cpp indexertest.cpp

indexertest_LDFLAGS = -lcxxtools
zimindexer_LDFLAGS = -ltntdb

LDADD = $(top_builddir)/src/zimlib/libzim.la
