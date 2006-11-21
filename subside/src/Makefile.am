INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = subside
subside_SOURCES           = subside_main.c
include_HEADERS           = subside.h

lib_LTLIBRARIES           = libsubside.la
libsubside_la_SOURCES     = subside.c

subside_LDADD             = -lsubside -lglib-2.0 -lm

