noinst_PROGRAMS = indexertest
bin_PROGRAMS = zimindexer

AM_CPPFLAGS=-I$(top_builddir)/include

zimindexer_SOURCES = articleparser.cpp zimindexer.cpp
indexertest_SOURCES = articleparser.cpp indexertest.cpp

indexertest_LDFLAGS = -lzim -lcxxtools
zimindexer_LDFLAGS = -lzim -ltntdb
