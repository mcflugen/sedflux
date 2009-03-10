include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(bing_SRCS
   bing_main.c
)

add_executable(run_bing ${bing_SRCS})

target_link_libraries(run_bing bing )

install(PROGRAMS run_bing DESTINATION bin RENAME bing)


########### next target ###############

SET(bing_LIB_SRCS
   bing.c
)

add_library(bing SHARED ${bing_LIB_SRCS})

target_link_libraries(bing sedflux)

install(TARGETS bing DESTINATION lib)


########### install files ###############

install(FILES  bing.h DESTINATION include/ew-2.0)



#original Makefile.am contents follow:

#INCLUDES               = -I$(top_srcdir)/ew
#
#AM_LDFLAGS             = $(top_builddir)/ew/utils/libutils.la \
#                         $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS           = bing
#bing_SOURCES           = bing_main.c
#bing_DEPENDENCIES      = libbing.la
#binginclude_HEADERS    = bing.h
#bingincludedir         = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES        = libbing.la
#libbing_la_SOURCES     = bing.c
#
#bing_LDADD             = -lbing -lglib-2.0
#
