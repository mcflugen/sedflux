include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

set (avulsion_SRCS avulsion_main.c)

add_executable (run_avulsion ${avulsion_SRCS})

target_link_libraries (run_avulsion avulsion sedflux)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_avulsion
       DESTINATION bin RENAME avulsion COMPONENT sedflux)


########### next target ###############

set (avulsion_LIB_SRCS avulsion.c avulsion_api.c)

set_source_files_properties (${avulsion_LIB_SRCS} PROPERTIES LANGUAGE CXX)
add_library(avulsion ${avulsion_LIB_SRCS})

target_link_libraries(avulsion sedflux)

install(TARGETS avulsion DESTINATION lib COMPONENT sedflux)


########### install files ###############

install (FILES avulsion.h avulsion_api.h
         DESTINATION include/ew-2.0
         COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS             = avulsion
#avulsion_SOURCES         = avulsion_main.c
#avulsion_DEPENDENCIES    = libavulsion.la
#avulsioninclude_HEADERS  = avulsion.h
#avulsionincludedir       = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES          = libavulsion.la
#libavulsion_la_SOURCES   = avulsion.c
#
#avulsion_LDADD           = -lavulsion -lglib-2.0
#
