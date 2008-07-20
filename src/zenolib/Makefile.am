AM_CPPFLAGS=-I$(top_builddir)/include

lib_LTLIBRARIES = libzeno.la
include_HEADERS = \
	Types.h \
	LzmaDec.h

libzeno_la_SOURCES = \
	article.cpp \
	articlebase.cpp \
	articlesearch.cpp \
	deflatestream.cpp \
	dirent.cpp \
	file.cpp \
	fileheader.cpp \
	fileimpl.cpp \
	files.cpp \
	indexarticle.cpp \
	inflatestream.cpp \
	qunicode.cpp \
	search.cpp \
	zintstream.cpp \
	LzmaDec.c

libzeno_la_LDFLAGS=-lcxxtools -ltntnet
