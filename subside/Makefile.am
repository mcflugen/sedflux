INCLUDES                 = -I$(top_srcdir)

AM_LDFLAGS               = $(top_builddir)/utils/libutils.la \
                           $(top_builddir)/sed/libsedflux.la

bin_PROGRAMS              = subside
subside_SOURCES           = subside_main.c
subsideinclude_HEADERS    = subside.h
subsideincludedir         = $(includedir)/ew-2.0

lib_LTLIBRARIES           = libsubside.la
libsubside_la_SOURCES     = subside.c

subside_LDADD             = -lsubside -lglib-2.0 -lm

