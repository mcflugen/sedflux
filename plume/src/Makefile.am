INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = plume plume_unit_test plume_run_file
plume_SOURCES             = plumemain.c plumeout1.c plumeread.c
plume_unit_test_SOURCES   = plume_unit_test.c
plume_run_file_SOURCES    = plume_run_file.c

lib_LTLIBRARIES           = libplume.la
libplume_la_SOURCES       = \
                            plume_approx.c  \
                            plume2d.c       \
                            plume.c         \
                            plumearray.c    \
                            plumecent.c     \
                            plumecheck.c    \
                            plumeconc.c     \
                            plumedist.c     \
                            plumefunct.c    \
                            plumejump.c     \
                            plumelog.c      \
                            plumemass.c     \
                            plumeout2.c     \
                            plumeout3.c     \
                            plumeread2d.c   \
                            plumeset.c

include_HEADERS           = plumevars.h plumeinput.h plume_types.h plume_approx.h

LDADD                     = -lplume 

