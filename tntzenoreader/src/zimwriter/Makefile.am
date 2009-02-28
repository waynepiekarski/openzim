bin_PROGRAMS = zimwriter
include_HEADERS = zimwriter.h

AM_CPPFLAGS=-I$(top_builddir)/include

zimwriter_SOURCES = zimwriter.cpp compressor.cpp zimcompressor.cpp

AM_LDFLAGS = -ltntdb
LDADD = $(top_builddir)/src/zimlib/libzim.la
