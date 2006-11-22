INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS       = bing
bing_SOURCES       = bing_main.c
include_HEADERS    = bing.h

lib_LTLIBRARIES    = libbing.la
libbing_la_SOURCES = bing.c

bing_LDADD         = -lbing -lglib-2.0

