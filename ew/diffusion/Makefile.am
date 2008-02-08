INCLUDES                 = -I$(top_srcdir)/ew

AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
                           $(top_builddir)/ew/sed/libsedflux.la

bin_PROGRAMS              = diffusion
diffusion_SOURCES         = diffusion_main.c
diffusion_DEPENDENCIES    = libdiffusion.la
diffusioninclude_HEADERS  = diffusion.h
diffusionincludedir       = $(includedir)/ew-2.0

lib_LTLIBRARIES           = libdiffusion.la
libdiffusion_la_SOURCES   = diffusion.c

diffusion_LDADD           = -ldiffusion -lglib-2.0


