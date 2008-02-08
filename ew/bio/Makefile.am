INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS             = bio
bio_SOURCES              = bio_main.c
bio_DEPENDENCIES         = libbio.la

lib_LTLIBRARIES          = libbio.la
libbio_la_SOURCES        = bio.c bio_sed_interface.c

bio_LDADD                = -lbio -lglib-2.0 -lm
bioinclude_HEADERS       = bio.h
bioincludedir            = $(includedir)/ew-2.0

