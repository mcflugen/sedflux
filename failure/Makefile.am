INCLUDES                 = -I$(top_srcdir)

AM_LDFLAGS               = $(top_builddir)/utils/libutils.la \
                           $(top_builddir)/sed/libsedflux.la

lib_LTLIBRARIES           = libfailure.la
libfailure_la_SOURCES     = failure.c getsurface.c decider.c
failureinclude_HEADERS    = failure.h
failureincludedir         = $(includedir)/ew-2.0

LDADD                     = -lfailure -lgthread-2.0 -lglib-2.0

