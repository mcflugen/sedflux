include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

set (failure_LIB_SRCS failure.c getsurface.c decider.c)

set_source_files_properties(${failure_LIB_SRCS} get_config_text.c
  PROPERTIES LANGUAGE CXX)
add_library(failure ${failure_LIB_SRCS} get_config_text.c)
add_library(failure-static STATIC ${failure_LIB_SRCS})

target_link_libraries(failure sedflux)

install(TARGETS failure DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  failure.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#lib_LTLIBRARIES           = libfailure.la
#libfailure_la_SOURCES     = failure.c getsurface.c decider.c
#failureinclude_HEADERS    = failure.h
#failureincludedir         = $(includedir)/ew-2.0
#
#LDADD                     = -lfailure -lgthread-2.0 -lglib-2.0
#
