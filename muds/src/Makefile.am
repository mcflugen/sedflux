INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = muds
muds_SOURCES              = muds_main.c
include_HEADERS           = muds.h

lib_LTLIBRARIES           = libmuds.la
libmuds_la_SOURCES        = muds.c

muds_LDADD = -lmuds -lglib-2.0

