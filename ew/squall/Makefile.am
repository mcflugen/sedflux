INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS              = squall

squall_SOURCES            = squall_main.c
squallinclude_HEADERS     = squall.h
squallincludedir          = $(includedir)/ew-2.0

lib_LTLIBRARIES           = libsquall.la
libsquall_la_SOURCES      = squall.c

squall_LDADD              = -lsquall -lglib-2.0

