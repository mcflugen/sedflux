INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS             = avulsion
avulsion_SOURCES         = avulsion_main.c
avulsioninclude_HEADERS  = avulsion.h
avulsionincludedir       = $(includedir)/ew-2.0

lib_LTLIBRARIES          = libavulsion.la
libavulsion_la_SOURCES   = avulsion.c

avulsion_LDADD           = -lavulsion -lglib-2.0

