AM_CPPFLAGS=-I$(top_builddir)/include

lib_LTLIBRARIES = libzeno.la
libzeno_la_SOURCES = \
	article.cpp \
	articlesearch.cpp \
	dirent.cpp file.cpp \
	fileimpl.cpp qunicode.cpp \
	inflatestream.cpp \
	search.cpp
libzeno_la_LDFLAGS=-lcxxtools -ltntnet
include_HEADERS = inflatestream.h
