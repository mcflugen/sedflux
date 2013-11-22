include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(subside_SRCS
   subside_main.c
)

add_executable(run_subside ${subside_SRCS})

target_link_libraries(run_subside subside-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_subside DESTINATION bin RENAME subside COMPONENT sedflux)


########### next target ###############

set (subside_LIB_SRCS subside.c subside_api.c)

set_source_files_properties(${subside_LIB_SRCS} get_config_text.c
  PROPERTIES LANGUAGE CXX)
add_library(subside ${subside_LIB_SRCS} get_config_text.c)
add_library(subside-static STATIC ${subside_LIB_SRCS})

target_link_libraries(subside sedflux)

install(TARGETS subside DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  subside.h subside_api.h
        DESTINATION include/ew-2.0
        COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = subside
#subside_SOURCES           = subside_main.c
#subside_DEPENDENCIES      = libsubside.la
#subsideinclude_HEADERS    = subside.h
#subsideincludedir         = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libsubside.la
#libsubside_la_SOURCES     = subside.c
#
#subside_LDADD             = -lsubside -lglib-2.0 -lm
#
#if ENABLE_CHECK
#
#bin_PROGRAMS                   += subside_unit_test
#subside_unit_test_SOURCES       = subside_unit_test.c
#subside_unit_test_DEPENDENCIES  = libsubside.la
#
#subside_unit_test_LDADD   = -lsubside -lglib-2.0 -lm @CHECK_LIBS@
#subside_unit_test_CFLAGS  = @CHECK_CFLAGS@
#
#endif
