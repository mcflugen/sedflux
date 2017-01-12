include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../sakura ${CMAKE_CURRENT_SOURCE_DIR}/../avulsion ${CMAKE_CURRENT_SOURCE_DIR}/../bio ${CMAKE_CURRENT_SOURCE_DIR}/../diffusion ${CMAKE_CURRENT_SOURCE_DIR}/../subside ${CMAKE_CURRENT_SOURCE_DIR}/../xshore ${CMAKE_CURRENT_SOURCE_DIR}/../squall ${CMAKE_CURRENT_SOURCE_DIR}/../flow ${CMAKE_CURRENT_SOURCE_DIR}/../muds ${CMAKE_CURRENT_SOURCE_DIR}/../bing ${CMAKE_CURRENT_SOURCE_DIR}/../failure ${CMAKE_CURRENT_SOURCE_DIR}/../quake ${CMAKE_CURRENT_SOURCE_DIR}/../compact ${CMAKE_CURRENT_SOURCE_DIR}/../inflow ${CMAKE_CURRENT_SOURCE_DIR}/../plume )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/sedflux.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sedflux.pc )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/sedflux2d.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sedflux2d.pc )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/sedflux3d.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sedflux3d.pc )
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/sedgrid.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sedgrid.pc )

set( sedflux_LIBS
  utils sedflux sakura avulsion bio diffusion subside xshore squall flow muds
  bing failure quake compact inflow plume)

set(sedflux_STATIC_LIBS
  utils-static sedflux-static sakura-static avulsion-static bio-static
  diffusion-static subside-static xshore-static squall-static flow-static
  muds-static bing-static failure-static quake-static compact-static
  inflow-static plume-static)

########### next target ###############

set(sedflux_SRCS
   main.c
)

add_executable(run_sedflux ${sedflux_SRCS})

target_link_libraries(run_sedflux sedflux-2.0-static ${sedflux_STATIC_LIBS}
  sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_sedflux DESTINATION bin RENAME sedflux COMPONENT sedflux)

# add_executable(run_sedgrid sedgrid_main.c)
# target_link_libraries(run_sedgrid bmi_sedgrid-static)
# install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_sedgrid DESTINATION bin RENAME sedgrid COMPONENT sedflux)

########### next target ###############

set (sedflux-2.0_LIB_SRCS
  sedflux_api.c
  sedflux_command_line.c 
  run_constants.c 
  run_squall.c 
  run_cpr.c 
  run_flow.c 
  rain_sediment.c 
  run_bbl.c 
  run_avulsion.c 
  run_measuring_station.c 
  run_data_dump.c 
  run_failure.c 
  run_isostasy.c 
  run_diffusion.c 
  run_xshore.c 
  run_storm.c 
  run_river.c 
  run_new_process.c 
  run_bedload.c 
  run_debris_flow.c 
  run_subsidence.c 
  run_compaction.c 
  run_erosion.c 
  run_sea_level.c 
  run_quake.c 
  run_tide.c 
  run_slump.c 
  run_turbidity_current.c 
  run_plume.c 
  sedflux.c 
  init.c 
  my_sedflux.c 
  sedflux_esmf.c
)

set_source_files_properties (${sedflux-2.0_LIB_SRCS} PROPERTIES LANGUAGE CXX)
add_library(sedflux-2.0 ${sedflux-2.0_LIB_SRCS})
add_library(sedflux-2.0-static STATIC ${sedflux-2.0_LIB_SRCS})
target_link_libraries(sedflux-2.0 ${sedflux_LIBS})

install(TARGETS sedflux-2.0 DESTINATION lib COMPONENT sedflux)
install(FILES  sedflux.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)
install(FILES  sedflux2d.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)
install(FILES  sedflux3d.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)
install(FILES  sedgrid.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)

### BMI lib ###

add_library(bmi_sedflux2d bmi_sedflux2d.c ${sedflux-2.0_LIB_SRCS})
add_library(bmi_sedflux2d-static STATIC bmi_sedflux2d.c ${sedflux-2.0_LIB_SRCS})
target_link_libraries(bmi_sedflux2d ${sedflux_LIBS})
target_link_libraries(bmi_sedflux2d-static ${sedflux_STATIC_LIBS})
set_target_properties(bmi_sedflux2d-static PROPERTIES OUTPUT_NAME
  bmi_sedflux2d)

install(TARGETS bmi_sedflux2d DESTINATION lib)
install(TARGETS bmi_sedflux2d-static DESTINATION lib)
install(FILES bmi_sedflux2d.h DESTINATION include/sedflux2d)
install(FILES bmi.h DESTINATION include/sedflux2d)

add_library(bmi_sedflux3d bmi_sedflux3d.c ${sedflux-2.0_LIB_SRCS})
add_library(bmi_sedflux3d-static STATIC bmi_sedflux3d.c ${sedflux-2.0_LIB_SRCS})
target_link_libraries(bmi_sedflux3d ${sedflux_LIBS})
target_link_libraries(bmi_sedflux3d-static ${sedflux_STATIC_LIBS})
set_target_properties(bmi_sedflux3d-static PROPERTIES OUTPUT_NAME
  bmi_sedflux3d)

install(TARGETS bmi_sedflux3d DESTINATION lib)
install(TARGETS bmi_sedflux3d-static DESTINATION lib)
install(FILES bmi_sedflux3d.h DESTINATION include/sedflux3d)
install(FILES bmi.h DESTINATION include/sedflux3d)

add_library(bmi_sedgrid bmi_sedgrid.c ${sedflux-2.0_LIB_SRCS})
add_library(bmi_sedgrid-static STATIC bmi_sedgrid.c ${sedflux-2.0_LIB_SRCS})
target_link_libraries(bmi_sedgrid ${sedflux_LIBS})
target_link_libraries(bmi_sedgrid-static ${sedflux_STATIC_LIBS})
set_target_properties(bmi_sedgrid-static PROPERTIES OUTPUT_NAME
  bmi_sedgrid)

install(TARGETS bmi_sedgrid DESTINATION lib)
install(TARGETS bmi_sedgrid-static DESTINATION lib)
install(FILES bmi_sedgrid.h DESTINATION include/sedgrid)
install(FILES bmi.h DESTINATION include/sedgrid)

########### install files ###############

install(FILES  sedflux.h sedflux_api.h DESTINATION include/ew-2.0 COMPONENT sedflux)
