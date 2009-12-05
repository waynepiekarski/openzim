SUBDIRS = \
	src \
	include

if WITH_CXXTOOLS
SUBDIRS += \
	zimDump

if MAKE_UNITTEST

SUBDIRS += \
	test

test: all
	test/zimlib-test

endif

endif
