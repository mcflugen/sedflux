include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(flow_LIB_SRCS
   flow.c
)

add_library(flow ${flow_LIB_SRCS} get_config_text.c)
add_library(flow-static STATIC ${flow_LIB_SRCS})

target_link_libraries(flow sedflux)

install(TARGETS flow DESTINATION lib COMPONENT sedflux)


########### next target ###############

set (flow_SRCS flow_main.c)

set_source_files_properties (${flow_LIB_SRCS} get_config_text.c
  PROPERTIES LANGUAGE CXX)
add_executable(run_flow ${flow_SRCS})

target_link_libraries(run_flow flow-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_flow DESTINATION bin RENAME flow COMPONENT sedflux)


########### next target ###############

SET(flow_2d_SRCS
   flow_2d_main.c
)

add_executable(flow_2d ${flow_2d_SRCS})

target_link_libraries(flow_2d flow-static sedflux-static)

install(TARGETS flow_2d DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(flow_3d_SRCS
   flow_3d_main.c
)

add_executable(flow_3d ${flow_3d_SRCS})

target_link_libraries(flow_3d flow-static sedflux-static)

install(TARGETS flow_3d DESTINATION bin COMPONENT sedflux)


########### install files ###############

install(FILES  flow.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = flow flow_2d flow_3d
#flow_SOURCES              = flow_main.c
#flow_2d_SOURCES           = flow_2d_main.c
#flow_3d_SOURCES           = flow_3d_main.c
#flow_DEPENDENCIES         = libflow.la
#flow_2d_DEPENDENCIES      = libflow.la
#flow_3d_DEPENDENCIES      = libflow.la
#flowinclude_HEADERS       = flow.h
#flowincludedir            = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES             = libflow.la
#libflow_la_SOURCES         = flow.c
#
#flow_LDADD                = -lflow -lglib-2.0
#flow_2d_LDADD             = -lflow -lglib-2.0 
#flow_3d_LDADD             = -lflow -lglib-2.0 
#
