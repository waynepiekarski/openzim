SUBDIRS = \
	src/zimlib \
	src/zimDump \
	include

if MAKE_TNTREADER
  SUBDIRS += \
  	src/webapp
endif

if MAKE_ZIMWRITER
  SUBDIRS += \
  	src/wikidownloader \
  	src/zimwriter \
  	src/zimindexer
endif

dist_bin_SCRIPTS = TntReaderConfig
EXTRA_DIST = tntnet.conf
