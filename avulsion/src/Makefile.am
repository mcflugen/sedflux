INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS             = avulsion
avulsion_SOURCES         = avulsion_main.c
include_HEADERS          = avulsion.h

lib_LTLIBRARIES          = libavulsion.la
libavulsion_la_SOURCES   = avulsion.c

avulsion_LDADD           = -lavulsion -lglib-2.0

