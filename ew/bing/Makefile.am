INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS        = bing
bing_SOURCES        = bing_main.c
binginclude_HEADERS = bing.h
bingincludedir      = $(includedir)/ew-2.0

lib_LTLIBRARIES     = libbing.la
libbing_la_SOURCES  = bing.c

bing_LDADD          = -lbing -lglib-2.0

