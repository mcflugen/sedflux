## Comment line

#bin_PROGRAMS = getsurface fos decider

lib_LTLIBRARIES = libfailure.la
libfailure_la_SOURCES = failure.c getsurface.c decider.c
include_HEADERS = failure.h

LDADD = -lfailure -lsedflux -lutils -lgthread-2.0 -lglib-2.0
#LDADD = -lthread -lsedflux -lutils -lglib-2.0 -lgthread-2.0 
#decider_LDADD = decider_standalone.o -lutils -lsedflux
#getsurface_LDADD = -lutils -lsedflux
#fos_LDADD = -lutils -lsedflux

#decider_standalone.o: decider.c
#	$(COMPILE) -DDECIDER_STANDALONE -o decider_standalone.o -c $(top_srcdir)/decider.c
#decider: decider_standalone.o
#	$(LINK)  decider_standalone.o $(LDADD) $(LIBS) -o decider


