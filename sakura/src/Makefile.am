INCLUDES                 = -I$(top_srcdir)/libutils/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la

bin_SCRIPTS               = runsakura
bin_PROGRAMS              = sakura

sakura_SOURCES            = sakura_main.c
include_HEADERS           = sakura.h sakura_utils.h

lib_LTLIBRARIES           = libsakura.la
libsakura_la_SOURCES      = sakura.c sakura_utils.c

LDADD                     = -lglib-2.0
sakura_LDADD              = sakura_standalone.o -lsakura -lglib-2.0

sakura_standalone.o: sakura.c
	$(COMPILE) -DSAKURA_STANDALONE -o sakura_standalone.o -c $(top_srcdir)/sakura/src/sakura.c

