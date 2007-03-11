SUFFIXES=.ecpp .gif .jpg .css .js .ico
.ecpp.cpp:
	ecppc $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

AM_CPPFLAGS=-I$(top_builddir)/include

tntnet_LTLIBRARIES = zeno.la
zeno_la_SOURCES = zenocomp.ecpp skin.cpp search.ecpp
zeno_la_LDFLAGS = -module -version-info @sonumber@ -ltntnet
zeno_la_LIBADD=$(top_builddir)/src/zenolib/libzeno.la

include_HEADERS = skin.h
