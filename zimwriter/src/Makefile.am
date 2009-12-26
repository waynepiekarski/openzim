bin_PROGRAMS = zimwriter

AM_CPPFLAGS=-I$(top_builddir)/include

zimwriter_SOURCES = \
    articleparser.cpp \
    articlesource.cpp \
    dbsource.cpp \
    filesource.cpp \
    indexersource.cpp \
    main.cpp \
    mstream.cpp \
    zimcreator.cpp \
    zimindexer.cpp

AM_LDFLAGS = -lzim -ltntdb
