INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_SCRIPTS              = runinflow
bin_PROGRAMS             = inflow pseudoplume

pseudoplume_SOURCES      = pseudoplume.c
include_HEADERS          = inflow.h

lib_LTLIBRARIES          = libinflow.la
libinflow_la_SOURCES     = inflow.c

LDADD                    = -lglib-2.0

inflow_SOURCES           = inflow_main.c inflow.c
inflow_CFLAGS            = -DINFLOW_STANDALONE
inflow_LDADD             = -lglib-2.0

inflow1d$(EXEEXT): inflow
	ln -s inflow inflow1d

