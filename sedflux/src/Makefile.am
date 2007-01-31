INCLUDES                 = -I$(top_srcdir)/libutils/src \
                           -I$(top_srcdir)/libsed/src \
                           -I$(top_srcdir)/sakura/src \
                           -I$(top_srcdir)/avulsion/src \
                           -I$(top_srcdir)/diffusion/src \
                           -I$(top_srcdir)/subside/src \
                           -I$(top_srcdir)/xshore/src \
                           -I$(top_srcdir)/squall/src \
                           -I$(top_srcdir)/flow/src \
                           -I$(top_srcdir)/muds/src \
                           -I$(top_srcdir)/bing/src \
                           -I$(top_srcdir)/failure/src \
                           -I$(top_srcdir)/quake/src \
                           -I$(top_srcdir)/compact/src \
                           -I$(top_srcdir)/inflow/src \
                           -I$(top_srcdir)/plume/src

AM_LDFLAGS               = $(top_builddir)/libutils/src/libutils.la \
                           $(top_builddir)/libsed/src/libsedflux.la \
                           $(top_builddir)/sakura/src/libsakura.la \
                           $(top_builddir)/avulsion/src/libavulsion.la \
                           $(top_builddir)/diffusion/src/libdiffusion.la \
                           $(top_builddir)/subside/src/libsubside.la \
                           $(top_builddir)/xshore/src/libxshore.la \
                           $(top_builddir)/squall/src/libsquall.la \
                           $(top_builddir)/flow/src/libflow.la \
                           $(top_builddir)/muds/src/libmuds.la \
                           $(top_builddir)/bing/src/libbing.la \
                           $(top_builddir)/failure/src/libfailure.la \
                           $(top_builddir)/quake/src/libquake.la \
                           $(top_builddir)/compact/src/libcompact.la \
                           $(top_builddir)/inflow/src/libinflow.la \
                           $(top_builddir)/plume/src/libplume.la

bin_SCRIPTS               = runsedflux runsedflux-batch
bin_PROGRAMS              = sedflux

sedflux_SOURCES           = \
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
                            main.c \
                            init.c

noinst_HEADERS            = cpr.h \
                            run_squall.h \
                            run_flow.h \
                            bbl.h \
                            run_diffusion.h \
                            bedload.h \
                            debris_flow.h \
                            data_dump.h \
                            sealevel.h \
                            isostasy.h \
                            river.h \
                            run_new_process.h \
                            measuring_station.h \
                            subsidence.h \
                            storms.h \
                            processes.h \
                            turbidity_current.h \
                            run_avulsion.h \
                            failure_proc.h \
                            compaction.h \
                            erosion.h \
                            run_plume.h \
                            quake.h \
                            tide.h \
                            slump.h \
                            run_constants.h \
                            run_xshore.h

install-exec-local:
	if test -h $(DESTDIR)$(bindir)/sedflux3d ; then \
	   rm $(DESTDIR)$(bindir)/sedflux3d ; \
	fi 
	ln -f -s $(DESTDIR)$(bindir)/sedflux $(DESTDIR)$(bindir)/sedflux3d

uninstall-local:
	if test -h $(DESTDIR)$(bindir)/sedflux3d ; then \
	   rm $(DESTDIR)$(bindir)/sedflux3d ; \
	fi



