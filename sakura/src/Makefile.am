INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = sakura

include_HEADERS           = sakura.h

lib_LTLIBRARIES           = libsakura.la
libsakura_la_SOURCES      = sakura.c sakura_io.c sakura_sedflux_interface.c sakura_utils.c

LDADD                     = -lglib-2.0

sakura_SOURCES            = sakura_main.c sakura.c sakura_io.c sakura_utils.c sakura_local.h sakura_utils.h
sakura_CFLAGS             = -DSAKURA_STANDALONE
sakura_LDADD              = -lglib-2.0

