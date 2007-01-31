SUFFIXES=.ecpp .gif .jpg .css .js
%.cpp: %.ecpp
	ecppc $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

AM_CPPFLAGS=-I$(top_builddir)/include

tntnet_LTLIBRARIES = zeno.la
zeno_la_SOURCES = zenocomp.cpp skin.cpp
zeno_la_LDFLAGS = -module -version-info @sonumber@ -ltntnet
zeno_la_LIBADD=$(top_builddir)/src/zenolib/libzeno.la
