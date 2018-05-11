include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(squall_SRCS
   squall_main.c
)

add_executable(run_squall ${squall_SRCS})

target_link_libraries(run_squall squall-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_squall DESTINATION bin RENAME squall COMPONENT sedflux)


########### next target ###############

set (squall_LIB_SRCS squall.c)

set_source_files_properties(${squall_LIB_SRCS} get_config_text.c
  PROPERTIES LANGUAGE C)
add_library(squall ${squall_LIB_SRCS} get_config_text.c)
add_library(squall-static STATIC ${squall_LIB_SRCS})

target_link_libraries(squall sedflux)

install(TARGETS squall DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  squall.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = squall
#
#squall_SOURCES            = squall_main.c
#squall_DEPENDENCIES       = libsquall.la
#squallinclude_HEADERS     = squall.h
#squallincludedir          = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libsquall.la
#libsquall_la_SOURCES      = squall.c
#
#squall_LDADD              = -lsquall -lglib-2.0
#
