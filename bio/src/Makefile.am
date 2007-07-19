INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS             = bio
bio_SOURCES              = bio_main.c

lib_LTLIBRARIES          = libbio.la
libbio_la_SOURCES        = bio.c bio_sed_interface.c

bio_LDADD                = -lbio -lglib-2.0 -lm

