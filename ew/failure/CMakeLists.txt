include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(failure_LIB_SRCS
   failure.c
   getsurface.c
   decider.c
)

add_library(failure SHARED ${failure_LIB_SRCS})

target_link_libraries(failure sedflux)

install(TARGETS failure DESTINATION lib)


########### install files ###############

install(FILES  failure.h DESTINATION include/ew-2.0)



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
