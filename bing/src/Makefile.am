## Comment line

bin_PROGRAMS = bing
bing_SOURCES = bing_main.c
include_HEADERS = bing.h

lib_LTLIBRARIES = libbing.la
libbing_la_SOURCES = bing.c

#lib_LIBRARIES = libbing.a
#libbing_a_SOURCES = bing.c

#LDADD = ../utils/libutils.a ../sed/libsedflux.a
bing_LDADD = -lbing -lsedflux -lutils -lglib-2.0

#noinst_HEADERS = bing.h

#bing.o: bing.c
#	$(COMPILE) -o bing.o -c $(top_srcdir)/bing.c
#
#bingStandalone.o: bing.c
#	$(COMPILE) -DBING_STANDALONE $(LIBS) -o bingStandalone.o -c $(top_srcdir)/bing.c
#
#bing: bingStandalone.o
#	$(LINK)  bingStandalone.o $(LIBS) $(bing_LDADD)
