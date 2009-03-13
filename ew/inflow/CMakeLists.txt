include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(inflow_SRCS
   inflow_main.c
)

add_executable(run_inflow ${inflow_SRCS})

target_link_libraries(run_inflow inflow)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_inflow DESTINATION bin RENAME inflow COMPONENT sedflux)


########### next target ###############

SET(pseudoplume_SRCS
   pseudoplume.c
)

add_executable(pseudoplume ${pseudoplume_SRCS})

target_link_libraries(pseudoplume inflow)

install(TARGETS pseudoplume DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(inflow_LIB_SRCS
  inflow.c
  inflow_io.c
  inflow_sedflux_interface.c
)

add_library(inflow ${inflow_LIB_SRCS})

target_link_libraries(inflow sedflux)

install(TARGETS inflow DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  inflow.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
##bin_SCRIPTS              = runinflow
#bin_PROGRAMS             = inflow pseudoplume
#
#pseudoplume_SOURCES      = pseudoplume.c
#pseudoplume_DEPENDENCIES = libinflow.la
#inflow_DEPENDENCIES      = libinflow.la
#inflowinclude_HEADERS    = inflow.h
#inflowincludedir         = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES          = libinflow.la
#libinflow_la_SOURCES     = inflow.c inflow_io.c inflow_sedflux_interface.c
#
#LDADD                    = -lglib-2.0
#
#inflow_SOURCES           = inflow_main.c inflow.c inflow_io.c inflow_local.h
#inflow_CFLAGS            = -DINFLOW_STANDALONE
#inflow_LDADD             = -lglib-2.0
#
#inflow1d$(EXEEXT): inflow
#	ln -s inflow inflow1d
#
