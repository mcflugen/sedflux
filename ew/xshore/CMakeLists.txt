include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(xshore_LIB_SRCS
   xshore.c
)

add_library(xshore ${xshore_LIB_SRCS})

target_link_libraries(xshore sedflux)

install(TARGETS xshore DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  xshore.h DESTINATION include/ew-2.0 COMPONENT sedflux)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#xshoreinclude_HEADERS     = xshore.h
#xshoreincludedir          = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libxshore.la
#libxshore_la_SOURCES      = xshore.c
#
