include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(squall_SRCS
   squall_main.c
)

add_executable(run_squall ${squall_SRCS})

target_link_libraries(run_squall squall)

install(PROGRAMS run_squall DESTINATION bin RENAME squall)


########### next target ###############

SET(squall_LIB_SRCS
   squall.c
)

add_library(squall SHARED ${squall_LIB_SRCS})

target_link_libraries(squall sedflux)

install(TARGETS squall DESTINATION lib)


########### install files ###############

install(FILES  squall.h DESTINATION include/ew-2.0)



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
