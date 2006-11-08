## Comment line

bin_SCRIPTS = runinflow
#bin_PROGRAMS = inflow inflow1d pseudoplume
bin_PROGRAMS = inflow pseudoplume
inflow_SOURCES = inflow_main.c
#inflow1d_SOURCES = 
pseudoplume_SOURCES = pseudoplume.c
include_HEADERS = inflow.h

lib_LTLIBRARIES = libinflow.la
libinflow_la_SOURCES = inflow.c

#lib_LIBRARIES = libinflow.a
#libinflow_a_SOURCES = inflow.c

LDADD = -lsedflux -lutils -lglib-2.0
inflow_LDADD = inflowStandalone.o -lsedflux -lutils -lglib-2.0

inflowStandalone.o: inflow.c
	$(COMPILE) -DINFLOW_STANDALONE -o inflowStandalone.o -c $(top_srcdir)/inflow.c

inflow1d$(EXEEXT): inflow
	ln -s inflow inflow1d

