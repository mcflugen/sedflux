include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(compact_SRCS
   compact_main.c
)

add_executable(run_compact ${compact_SRCS})

target_link_libraries(run_compact compact-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_compact DESTINATION bin RENAME compact COMPONENT sedflux)


########### next target ###############

set (compact_LIB_SRCS compact.c)

set_source_files_properties (${compact_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(compact ${compact_LIB_SRCS})
add_library(compact-static STATIC ${compact_LIB_SRCS})

target_link_libraries(compact sedflux)

install(TARGETS compact DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  compact.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = compact
#compact_SOURCES           = compact_main.c
#compact_DEPENDENCIES      = libcompact.la
#compactinclude_HEADERS    = compact.h
#compactincludedir         = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libcompact.la
#libcompact_la_SOURCES     = compact.c
#
#compact_LDADD             = -lcompact -lglib-2.0 -lm
#
#if ENABLE_CHECK
#
#bin_PROGRAMS                   += compact_unit_test
#compact_unit_test_SOURCES       = compact_unit_test.c
#compact_unit_test_DEPENDENCIES  = libcompact.la
#
#compact_unit_test_LDADD   = -lcompact -lglib-2.0 -lm @CHECK_LIBS@
#compact_unit_test_CFLAGS  = @CHECK_CFLAGS@
#
#endif
#
