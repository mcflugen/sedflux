include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

set (earthquake_SRCS earthquake_main.c)

add_executable(run_earthquake ${earthquake_SRCS})

target_link_libraries(run_earthquake quake-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_earthquake DESTINATION bin RENAME earthquake COMPONENT sedflux)


########### next target ###############

set (quake_LIB_SRCS earthquake.c)

set_source_files_properties (${quake_LIB_SRCS} PROPERTIES LANGUAGE CXX)
add_library(quake ${quake_LIB_SRCS})
add_library(quake-static STATIC ${quake_LIB_SRCS})

target_link_libraries(quake utils)

install(TARGETS quake DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES earthquake.h DESTINATION include/ew-2.0 COMPONENT sedflux)


#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la
#
#bin_PROGRAMS             = earthquake 
#earthquake_SOURCES       = earthquake_main.c
#earthquake_DEPENDENCIES  = libquake.la
#
#lib_LTLIBRARIES          = libquake.la
#libquake_la_SOURCES      = earthquake.c
#
#LDADD                    = -lquake -lglib-2.0
#
#quakeconvert_to$(EXEEXT): earthquake
#	ln -s earthquake quakeconvert_to
#
#quakeconvert_from$(EXEEXT): earthquake
#	ln -s earthquake quakeconvert_from
#
