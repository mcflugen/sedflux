
########### next target ###############

SET(hydrotrend_SRCS
   hydrotrend.c
   hydroalloc_mem.c
   hydrocalqsnew.c
   hydrocheckinput.c
   hydroclimate.c
   hydrocommandline.c
   hydroexpdist.c
   hydrofree_mem.c
   hydrofunct.c
   hydroglacial.c
   hydrohypsom.c
   hydromaxevents.c
   hydroopenfiles.c
   hydrooutlet.c
   hydrooutput.c
   hydroprintannual.c
   hydroprintstat.c
   hydroprinttable.c
   hydrorain.c
   hydroran2.c
   hydrorandom.c
   hydroreadclimate.c
   hydroreadhypsom.c
   hydroreadinput.c
   hydrosecurityinputcheck.c
   hydrosedload.c
   hydrosetgeoparams.c
   hydrosetglobalpar.c
   hydrosetparams.c
   hydroshoulder.c
   hydroshuffle.c
   hydrosnow.c
   hydrosumflow.c
   hydroswap.c
   hydroweather.c
)

add_executable(run_hydrotrend ${hydrotrend_SRCS})

target_link_libraries(run_hydrotrend)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_hydrotrend
  DESTINATION bin
  RENAME hydrotrend
)
