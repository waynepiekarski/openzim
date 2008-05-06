bin_PROGRAMS = getarticles dumparticle trycompress

AM_CPPFLAGS=-I$(top_builddir)/include

getarticles_SOURCES = getarticles.cpp
dumparticle_SOURCES = dumparticle.cpp
trycompress_SOURCES = trycompress.cpp

LDFLAGS = -ltntnet -ltntdb
LDADD = $(top_builddir)/src/zenolib/libzeno.la

