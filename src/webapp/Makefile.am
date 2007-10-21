SUFFIXES=.ecpp .gif .jpg .css .js .ico
.ecpp.cpp:
	ecppc $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<
.png.cpp:
	ecppc $(ECPPFLAGS) -b $(ECPPFLAGS_PNG) $<
.js.cpp:
	ecppc $(ECPPFLAGS) -b $(ECPPFLAGS_JS) $<

AM_CPPFLAGS=-I$(top_builddir)/include

bin_PROGRAMS = TntReader

TntReader_SOURCES = zenocomp.ecpp skin.ecpp \
	search.ecpp searcharticles.ecpp searchresults.ecpp article.ecpp \
	tntnet_png.png random.ecpp notfound.ecpp number.ecpp \
	backgroundreader.cpp backgroundreader.h pager.ecpp \
	browse.ecpp browsescreen.ecpp browseresults.ecpp \
	ajax_js.js redirect.ecpp main.cpp

TntReader_LDFLAGS = -ltntnet
TntReader_LDADD = $(top_builddir)/src/zenolib/libzeno.la
