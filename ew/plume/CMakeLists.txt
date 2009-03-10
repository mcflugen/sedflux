include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(plume_SRCS
   plumemain.c
)

add_executable(run_plume ${plume_SRCS})

target_link_libraries(run_plume plume)

install(PROGRAMS run_plume DESTINATION bin RENAME plume)


########### next target ###############

SET(plume_run_file_SRCS
   plume_run_file.c
)

add_executable(plume_run_file ${plume_run_file_SRCS})

target_link_libraries(plume_run_file plume)

install(TARGETS plume_run_file DESTINATION bin)


########### next target ###############

SET(plume_LIB_SRCS
   plume_scan.c
   plume_approx.c
   plumeout1.c
   plumeread.c
   plume2d.c
   plume.c
   plumearray.c
   plumecent.c
   plumecheck.c
   plumeconc.c
   plumedist.c
   plumefunct.c
   plumejump.c
   plumelog.c
   plumemass.c
   plumeout2.c
   plumeout3.c
   plumeread2d.c
   plumeset.c
)

add_library(plume SHARED ${plume_LIB_SRCS})

target_link_libraries(plume sedflux)

install(TARGETS plume DESTINATION lib)


########### install files ###############

install(FILES  plumevars.h plumeinput.h plume_types.h plume_approx.h plume_local.h DESTINATION include/ew-2.0)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS                = plume plume_run_file
#plume_SOURCES               = plumemain.c plumeout1.c plumeread.c plume_scan.c
#plume_run_file_SOURCES      = plume_run_file.c
#plume_DEPENDENCIES          = libplume.la
#plume_run_file_DEPENDENCIES = libplume.la
#
#if ENABLE_CHECK
#
#bin_PROGRAMS                 += plume_unit_test
#plume_unit_test_SOURCES       = plume_unit_test.c
#plume_unit_test_DEPENDENCIES  = libplume.la
#
#plume_unit_test_LDADD     = @CHECK_LIBS@ -lplume
#plume_unit_test_CFLAGS    = @CHECK_CFLAGS@
#endif
#
#lib_LTLIBRARIES           = libplume.la
#libplume_la_SOURCES       = \
#                            plume_approx.c  \
#                            plume2d.c       \
#                            plume.c         \
#                            plumearray.c    \
#                            plumecent.c     \
#                            plumecheck.c    \
#                            plumeconc.c     \
#                            plumedist.c     \
#                            plumefunct.c    \
#                            plumejump.c     \
#                            plumelog.c      \
#                            plumemass.c     \
#                            plumeout2.c     \
#                            plumeout3.c     \
#                            plumeread2d.c   \
#                            plumeset.c
#
#plumeinclude_HEADERS      = plumevars.h plumeinput.h plume_types.h plume_approx.h plume_local.h
#plumeincludedir           = $(includedir)/ew-2.0
#
#LDADD                     = -lplume 
#
