noinst_PROGRAMS = indexertest
bin_PROGRAMS = zenoindexer
#include_HEADERS = zenoindexer.h

AM_CPPFLAGS=-I$(top_builddir)/include

zenoindexer_SOURCES = articleparser.cpp zenoindexer.cpp
indexertest_SOURCES = articleparser.cpp indexertest.cpp

indexertest_LDFLAGS = -lcxxtools
zenoindexer_LDFLAGS = -ltntdb

LDADD = $(top_builddir)/src/zenolib/libzeno.la
