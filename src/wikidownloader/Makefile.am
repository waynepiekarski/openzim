bin_PROGRAMS = getarticles putarticle

AM_CPPFLAGS=-I$(top_builddir)/include

getarticles_SOURCES = getarticles.cpp
putarticle_SOURCES = putarticle.cpp

AM_LDFLAGS = -ltntnet -ltntdb
LDADD = $(top_builddir)/src/zenolib/libzeno.la

