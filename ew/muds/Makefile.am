INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS              = muds
muds_SOURCES              = muds_main.c
muds_DEPENDENCIES         = libmuds.la
mudsinclude_HEADERS       = muds.h
mudsincludedir            = $(includedir)/ew-2.0

lib_LTLIBRARIES           = libmuds.la
libmuds_la_SOURCES        = muds.c

muds_LDADD = -lmuds -lglib-2.0

