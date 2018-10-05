include_directories(${CMAKE_SOURCE_DIR}/ew )

set(PLUME_VERSION "0.1")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/plume.pc.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/plume.pc
)

########### next target ###############

SET(plume_SRCS plumemain.c)

add_executable(run_plume ${plume_SRCS})

target_link_libraries(run_plume bmi_plume sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_plume
  DESTINATION bin
  RENAME plume
  COMPONENT sedflux
)

########### next target ###############

SET(plume_run_file_SRCS plume_run_file.c)

add_executable(plume_run_file ${plume_run_file_SRCS})

target_link_libraries(plume_run_file plume-static sedflux-static)

install(TARGETS plume_run_file DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(plume_LIB_SRCS
  plume_scan.c
  plume_approx.c
  plumeout1.c
  plumeread.c
  plume2d.c
  plume.c
  plumearray.c
  plumecent.c
  plumecheck.c
  plumeconc.c
  plumedist.c
  plumefunct.c
  plumejump.c
  plumelog.c
  plumemass.c
  plumeout2.c
  plumeout3.c
  plumeread2d.c
  plumeset.c
)

set_source_files_properties(
  ${plume_LIB_SRCS}
  get_config_text.c
  PROPERTIES LANGUAGE C
)
add_library(plume ${plume_LIB_SRCS} get_config_text.c)
add_library(plume-static STATIC ${plume_LIB_SRCS})

target_link_libraries(plume sedflux)

install(TARGETS plume DESTINATION lib COMPONENT sedflux)

########### install files ###############

install(
  FILES
    plumevars.h
    plumeinput.h
    plume_types.h
    plume_approx.h
    plume_local.h
  DESTINATION include/ew-2.0
  COMPONENT sedflux
)

install(FILES plume.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)

### BMI

set(bmi_lib_sources bmi_plume.c plume_model.c)

add_library(bmi_plume ${bmi_lib_sources})
add_library(bmi_plume-static STATIC ${bmi_lib_sources})

target_link_libraries(bmi_plume plume sedflux)
target_link_libraries(bmi_plume-static plume-static sedflux-static)
set_target_properties(bmi_plume-static PROPERTIES OUTPUT_NAME bmi_plume)

install(TARGETS bmi_plume DESTINATION lib)
install(TARGETS bmi_plume-static DESTINATION lib)
install(FILES bmi_plume.h DESTINATION include/plume)
install(FILES bmi.h DESTINATION include/plume)
