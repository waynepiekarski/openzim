SUBDIRS = \
	src/zenolib \
	src/zenoDump \
	src/webapp \
	include

SKIN_FILES =  \
	skin/skin.css \
	skin/skin.html \
	skin/tntnet.jpg \
	skin/Wiki.png

EXTRA_DIST = tntnet.conf tntnet.properties TntReader \
	$(SKIN_FILES)

skin.zip: $(SKIN_FILES)
	(cd skin && zip ../skin.zip *)
