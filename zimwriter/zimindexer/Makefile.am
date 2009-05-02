noinst_PROGRAMS = indexertest mstreamtest
bin_PROGRAMS = zimindexer

AM_CPPFLAGS=-I$(top_builddir)/include

zimindexer_SOURCES = articleparser.cpp zimindexer.cpp
indexertest_SOURCES = articleparser.cpp indexertest.cpp
mstreamtest_SOURCES = mstream.cpp mstreamtest.cpp

indexertest_LDFLAGS = -lzim -lcxxtools
zimindexer_LDFLAGS = -lzim -ltntdb
