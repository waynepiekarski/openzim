SUFFIXES=.ecpp .gif .jpg .css .js .ico
.ecpp.cpp:
	ecppc $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<
.png.cpp:
	ecppc $(ECPPFLAGS) -b $(ECPPFLAGS_PNG) $<

AM_CPPFLAGS=-I$(top_builddir)/include

tntnet_LTLIBRARIES = zeno.la

zeno_la_SOURCES = zenocomp.ecpp skin.ecpp \
	search.ecpp searcharticles.ecpp searchresults.ecpp article.ecpp \
	tntnet_png.png random.ecpp notfound.ecpp number.ecpp \
	backgroundreader.cpp backgroundreader.h

zeno_la_LDFLAGS = -module -version-info @sonumber@ -ltntnet
zeno_la_LIBADD=$(top_builddir)/src/zenolib/libzeno.la
