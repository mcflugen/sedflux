include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(diffusion_SRCS
   diffusion_main.c
)

add_executable(run_diffusion ${diffusion_SRCS})

target_link_libraries(run_diffusion diffusion)

install(PROGRAMS run_diffusion DESTINATION bin RENAME diffusion)


########### next target ###############

SET(diffusion_LIB_SRCS
   diffusion.c
)

add_library(diffusion SHARED ${diffusion_LIB_SRCS})

target_link_libraries(diffusion sedflux)

install(TARGETS diffusion DESTINATION lib)


########### install files ###############

install(FILES  diffusion.h DESTINATION include/ew-2.0)



#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
#bin_PROGRAMS              = diffusion
#diffusion_SOURCES         = diffusion_main.c
#diffusion_DEPENDENCIES    = libdiffusion.la
#diffusioninclude_HEADERS  = diffusion.h
#diffusionincludedir       = $(includedir)/ew-2.0
#
#lib_LTLIBRARIES           = libdiffusion.la
#libdiffusion_la_SOURCES   = diffusion.c
#
#diffusion_LDADD           = -ldiffusion -lglib-2.0
#
#
