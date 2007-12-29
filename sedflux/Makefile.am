INCLUDES                 = -I$(top_srcdir) \
                           -I$(top_srcdir)/sakura \
                           -I$(top_srcdir)/avulsion \
                           -I$(top_srcdir)/bio \
                           -I$(top_srcdir)/diffusion \
                           -I$(top_srcdir)/subside \
                           -I$(top_srcdir)/xshore \
                           -I$(top_srcdir)/squall \
                           -I$(top_srcdir)/flow \
                           -I$(top_srcdir)/muds \
                           -I$(top_srcdir)/bing \
                           -I$(top_srcdir)/failure \
                           -I$(top_srcdir)/quake \
                           -I$(top_srcdir)/compact \
                           -I$(top_srcdir)/inflow \
                           -I$(top_srcdir)/plume

AM_LDFLAGS               = $(top_builddir)/utils/libutils.la \
                           $(top_builddir)/sed/libsedflux.la \
                           $(top_builddir)/sakura/libsakura.la \
                           $(top_builddir)/avulsion/libavulsion.la \
                           $(top_builddir)/bio/libbio.la \
                           $(top_builddir)/diffusion/libdiffusion.la \
                           $(top_builddir)/subside/libsubside.la \
                           $(top_builddir)/xshore/libxshore.la \
                           $(top_builddir)/squall/libsquall.la \
                           $(top_builddir)/flow/libflow.la \
                           $(top_builddir)/muds/libmuds.la \
                           $(top_builddir)/bing/libbing.la \
                           $(top_builddir)/failure/libfailure.la \
                           $(top_builddir)/quake/libquake.la \
                           $(top_builddir)/compact/libcompact.la \
                           $(top_builddir)/inflow/libinflow.la \
                           $(top_builddir)/plume/libplume.la

bin_SCRIPTS               = runsedflux sedflux-run-batch
bin_PROGRAMS              = sedflux
sedflux_LDADD             = -lsedflux-2.0 -lgthread-2.0 -lglib-2.0

lib_LTLIBRARIES           = libsedflux-2.0.la

libsedflux_2_0_la_SOURCES = \
                            sedflux_command_line.c \
                            run_constants.c \
                            run_squall.c \
                            run_cpr.c \
                            run_flow.c \
                            rain_sediment.c \
                            run_bbl.c \
                            run_avulsion.c \
                            run_measuring_station.c \
                            run_data_dump.c \
                            run_failure.c \
                            run_isostasy.c \
                            run_diffusion.c \
                            run_xshore.c \
                            run_storm.c \
                            run_river.c \
                            run_new_process.c \
                            run_bedload.c \
                            run_debris_flow.c \
                            run_subsidence.c \
                            run_compaction.c \
                            run_erosion.c \
                            run_sea_level.c \
                            run_quake.c \
                            run_tide.c \
                            run_slump.c \
                            run_turbidity_current.c \
                            run_plume.c \
                            sedflux.c \
                            init.c \
                            my_sedflux.c \
                            sedflux_esmf.c

EXTRA_DIST                = sedflux_esmf.F90

sedflux_SOURCES           = main.c


sedfluxinclude_HEADERS    = sedflux.h
sedfluxincludedir         = $(includedir)/ew-2.0

noinst_HEADERS            = \
                            my_sedflux.h \
                            my_processes.h

#noinst_HEADERS            = \
#                            cpr.h \
#                            run_squall.h \
#                            run_flow.h \
#                            bbl.h \
#                            run_diffusion.h \
#                            bedload.h \
#                            debris_flow.h \
#                            data_dump.h \
#                            sealevel.h \
#                            isostasy.h \
#                            river.h \
#                            run_new_process.h \
#                            measuring_station.h \
#                            subsidence.h \
#                            storms.h \
#                            processes.h \
#                            turbidity_current.h \
#                            run_avulsion.h \
#                            failure_proc.h \
#                            compaction.h \
#                            erosion.h \
#                            run_plume.h \
#                            quake.h \
#                            tide.h \
#                            slump.h \
#                            run_constants.h \
#                            run_xshore.h \
#                            my_sedflux.h

#install-exec-local:
#	if test -h $(DESTDIR)$(bindir)/sedflux3d ; then \
#	   rm $(DESTDIR)$(bindir)/sedflux3d ; \
#	fi 
#	ln -f -s $(DESTDIR)$(bindir)/sedflux $(DESTDIR)$(bindir)/sedflux3d

#uninstall-local:
#	if test -h $(DESTDIR)$(bindir)/sedflux3d ; then \
#	   rm $(DESTDIR)$(bindir)/sedflux3d ; \
#	fi



