bin_PROGRAMS = zenowriter
include_HEADERS = zenowriter.h

AM_CPPFLAGS=-I$(top_builddir)/include

zenowriter_SOURCES = zenowriter.cpp compressor.cpp zenocompressor.cpp

AM_LDFLAGS = -ltntdb
LDADD = $(top_builddir)/src/zenolib/libzeno.la
