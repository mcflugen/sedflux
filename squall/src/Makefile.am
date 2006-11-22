INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = squall

squall_SOURCES            = squall_main.c
include_HEADERS           = squall.h

lib_LTLIBRARIES           = libsquall.la
libsquall_la_SOURCES      = squall.c

squall_LDADD              = -lsquall -lglib-2.0

