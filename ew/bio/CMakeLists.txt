include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(bio_SRCS
   bio_main.c
)

add_executable(run_bio ${bio_SRCS})

target_link_libraries(run_bio bio)

install(PROGRAMS run_bio DESTINATION bin RENAME bio)


########### next target ###############

SET(bio_LIB_SRCS
   bio.c
   bio_sed_interface.c
)

add_library(bio SHARED ${bio_LIB_SRCS})

target_link_libraries(bio sedflux)

install(TARGETS bio DESTINATION lib)


########### install files ###############

install(FILES  bio.h DESTINATION include/ew-2.0)



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
