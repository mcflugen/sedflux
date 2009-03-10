include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(sakura_SRCS sakura_main.c)

add_executable(run_sakura ${sakura_SRCS})

target_link_libraries(run_sakura sakura)

install(PROGRAMS run_sakura DESTINATION bin RENAME sakura)


########### next target ###############

SET(sakura_LIB_SRCS
   sakura.c
   sakura_io.c
   sakura_sedflux_interface.c
   sakura_utils.c
)

add_library(sakura SHARED ${sakura_LIB_SRCS})

target_link_libraries(sakura sedflux)

install(TARGETS sakura DESTINATION lib)


########### install files ###############

install(FILES  sakura.h DESTINATION include/ew-2.0)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = sakura
#
#sakurainclude_HEADERS     = sakura.h
#sakuraincludedir          = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libsakura.la
#libsakura_la_SOURCES      = sakura.c sakura_io.c sakura_sedflux_interface.c sakura_utils.c
#
#LDADD                     = -lglib-2.0
#
#sakura_SOURCES            = sakura_main.c sakura.c sakura_io.c sakura_utils.c sakura_local.h sakura_utils.h
#sakura_DEPENDENCIES       = libsakura.la
#sakura_CFLAGS             = -DSAKURA_STANDALONE
#sakura_LDADD              = -lglib-2.0
#
#if ENABLE_CHECK
#
#bin_PROGRAMS                  += sakura_unit_test
#sakura_unit_test_SOURCES       = sakura_unit_test.c
#sakura_unit_test_DEPENDENCIES  = libsakura.la
#
#sakura_unit_test_LDADD         = -lsakura -lglib-2.0 -lm @CHECK_LIBS@
#sakura_unit_test_CFLAGS        = @CHECK_CFLAGS@
#
#endif
#
