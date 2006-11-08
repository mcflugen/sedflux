## Comment line

bin_SCRIPTS = runsakura
bin_PROGRAMS = sakura
#sakura_SOURCES = sakura_main.c sakura.c sakura_utils.c
#sakura_SOURCES = sakura_main.c sakura_utils.c
sakura_SOURCES = sakura_main.c
include_HEADERS = sakura.h sakura_utils.h

lib_LTLIBRARIES = libsakura.la
libsakura_la_SOURCES = sakura.c sakura_utils.c

#lib_LIBRARIES = libsakura.a
#libsakura_a_SOURCES = sakura.c sakura_utils.c

LDADD = -lutils -lglib-2.0
sakura_LDADD = sakura_standalone.o -lsakura -lutils -lglib-2.0

sakura_standalone.o: sakura.c
	$(COMPILE) -DSAKURA_STANDALONE -o sakura_standalone.o -c $(top_srcdir)/sakura.c

