INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_builddir)/libsed/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la

bin_PROGRAMS              = diffusion
diffusion_SOURCES         = diffusion_main.c
include_HEADERS           = diffusion.h

lib_LTLIBRARIES           = libdiffusion.la
libdiffusion_la_SOURCES   = diffusion.c

diffusion_LDADD           = -ldiffusion -lglib-2.0


