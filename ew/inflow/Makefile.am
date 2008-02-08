INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

#bin_SCRIPTS              = runinflow
bin_PROGRAMS             = inflow pseudoplume

pseudoplume_SOURCES      = pseudoplume.c
pseudoplume_DEPENDENCIES = libinflow.la
inflow_DEPENDENCIES      = libinflow.la
inflowinclude_HEADERS    = inflow.h
inflowincludedir         = $(includedir)/ew-2.0

lib_LTLIBRARIES          = libinflow.la
libinflow_la_SOURCES     = inflow.c inflow_io.c inflow_sedflux_interface.c

LDADD                    = -lglib-2.0

inflow_SOURCES           = inflow_main.c inflow.c inflow_io.c inflow_local.h
inflow_CFLAGS            = -DINFLOW_STANDALONE
inflow_LDADD             = -lglib-2.0

inflow1d$(EXEEXT): inflow
	ln -s inflow inflow1d

