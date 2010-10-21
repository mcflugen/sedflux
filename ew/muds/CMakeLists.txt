include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(muds_SRCS
   muds_main.c
)

add_executable(run_muds ${muds_SRCS})

target_link_libraries(run_muds muds)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_muds DESTINATION bin RENAME muds COMPONENT sedflux)


########### next target ###############

set (muds_LIB_SRCS muds.c)

set_source_files_properties (${muds_LIB_SRCS} PROPERTIES LANGUAGE CXX)
add_library(muds ${muds_LIB_SRCS})

target_link_libraries(muds sedflux)

install(TARGETS muds DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  muds.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = muds
#muds_SOURCES              = muds_main.c
#muds_DEPENDENCIES         = libmuds.la
#mudsinclude_HEADERS       = muds.h
#mudsincludedir            = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libmuds.la
#libmuds_la_SOURCES        = muds.c
#
#muds_LDADD = -lmuds -lglib-2.0
#
