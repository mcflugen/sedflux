## Comment line

bin_SCRIPTS = runsedflux runsedflux-batch
bin_PROGRAMS = sedflux sedflux3d
#bin_PROGRAMS = sedflux
sedflux3d_SOURCES = 
sedflux_SOURCES = \
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
   main_3d.c \
   main.c \
   init.c

noinst_HEADERS = cpr.h run_squall.h run_flow.h bbl.h run_diffusion.h \
   bedload.h debris_flow.h data_dump.h sealevel.h isostasy.h river.h         \
   measuring_station.h subsidence.h storms.h processes.h turbidity_current.h \
   avulsion.h failure_proc.h compaction.h erosion.h run_plume.h quake.h      \
   tide.h slump.h run_constants.h run_xshore.h

# -lthread
LDADD= \
 -lsakura \
 -ldiffusion \
 -lsubside \
 -lxshore \
 -lsquall \
 -lflow \
 -lmuds \
 -lbing \
 -lfailure \
 -lquake \
 -lcompact \
 -linflow \
 -lplume \
 -lsedflux \
 -lutils \
 -lgthread-2.0 \
 -lglib-2.0

sedflux3d$(EXEEXT): sedflux$(EXEEXT)
	ln -f -s sedflux$(EXEEXT) sedflux3d$(EXEEXT)
