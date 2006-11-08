## Comment line

bin_PROGRAMS = plume plume_unit_test plume_run_file
plume_SOURCES = plumemain.c plumeout1.c plumeread.c
plume_unit_test_SOURCES = plume_unit_test.c
plume_run_file_SOURCES = plume_run_file.c

#plume_SOURCES =          \
#        plume.c         \
#        plumearray.c    \
#        plumecent.c     \
#        plumecheck.c    \
#        plumeconc.c     \
#        plumedist.c     \
#        plumefunct.c    \
#        plumejump.c     \
#        plumelog.c      \
#        plumemass.c     \
#        plumeout1.c     \
#        plumeread.c     \
#        plumeset.c      \
#        plumemain.c

lib_LTLIBRARIES = libplume.la
libplume_la_SOURCES =     \
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

include_HEADERS = plumevars.h plumeinput.h plume_types.h plume_approx.h

LDADD = -lplume -lsedflux -lutils

