SUBDIRS = \
	src/zenolib \
	src/zenoDump \
	include

if MAKE_TNTREADER
  SUBDIRS += \
  	src/webapp
endif

if MAKE_ZENOWRITER
  SUBDIRS += \
  	src/wikidownloader \
  	src/zenowriter \
  	src/zenoindexer
endif

dist_bin_SCRIPTS = TntReaderConfig
EXTRA_DIST = tntnet.conf
