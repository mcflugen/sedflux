include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(bio_SRCS
   bio_main.c
)

add_executable(run_bio ${bio_SRCS})

target_link_libraries(run_bio bio-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_bio DESTINATION bin RENAME bio COMPONENT sedflux)


########### next target ###############

set(bio_LIB_SRCS bio.c bio_sed_interface.c)

set_source_files_properties(${bio_LIB_SRCS} get_config_text.c
  PROPERTIES LANGUAGE C)
add_library(bio ${bio_LIB_SRCS} get_config_text.c)
add_library(bio-static STATIC ${bio_LIB_SRCS})

target_link_libraries(bio sedflux)

install(TARGETS bio DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  bio.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS             = bio
#bio_SOURCES              = bio_main.c
#bio_DEPENDENCIES         = libbio.la
#
#lib_LTLIBRARIES          = libbio.la
#libbio_la_SOURCES        = bio.c bio_sed_interface.c
#
#bio_LDADD                = -lbio -lglib-2.0 -lm
#bioinclude_HEADERS       = bio.h
#bioincludedir            = $(includedir)/ew-2.0
#
