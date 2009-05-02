bin_PROGRAMS = zimwriter
include_HEADERS = zimwriter.h

AM_CPPFLAGS=-I$(top_builddir)/include

zimwriter_SOURCES = main.cpp dbsource.cpp filesource.cpp zimcreator.cpp articleparser.cpp zimindexer.cpp indexersource.cpp mstream.cpp articlesource.cpp

AM_LDFLAGS = -lzim -ltntdb
