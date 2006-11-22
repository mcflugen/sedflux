INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

lib_LTLIBRARIES           = libfailure.la
libfailure_la_SOURCES     = failure.c getsurface.c decider.c
include_HEADERS           = failure.h

LDADD                     = -lfailure -lgthread-2.0 -lglib-2.0

