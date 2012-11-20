include_directories(${CMAKE_SOURCE_DIR}/ew )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/plume.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/plume.pc )

########### next target ###############

SET(plume_SRCS
   plumemain.c
)

add_executable(run_plume ${plume_SRCS})

#target_link_libraries(run_plume plume plume_bmi)
target_link_libraries(run_plume plume)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_plume DESTINATION bin RENAME plume COMPONENT sedflux)


########### next target ###############

SET(plume_run_file_SRCS
   plume_run_file.c
)

add_executable(plume_run_file ${plume_run_file_SRCS})

target_link_libraries(plume_run_file plume)

install(TARGETS plume_run_file DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(plume_LIB_SRCS
   plume_bmi.c
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

set_source_files_properties (${plume_LIB_SRCS} PROPERTIES LANGUAGE CXX)
add_library(plume ${plume_LIB_SRCS})

target_link_libraries(plume sedflux)

install(TARGETS plume DESTINATION lib COMPONENT sedflux)

set (plume_bmi_LIB_SRCS
  plume_bmi.c)

set_source_files_properties (${plume_bmi_LIB_SRCS} PROPERTIES LANGUAGE CXX)

add_library(plume_bmi ${plume_bmi_LIB_SRCS})
target_link_libraries(plume_bmi sedflux)
install(TARGETS plume_bmi DESTINATION lib)

########### install files ###############

install(FILES  plumevars.h plumeinput.h plume_types.h plume_approx.h plume_local.h DESTINATION include/ew-2.0 COMPONENT sedflux)
install (FILES plume_bmi.h RENAME bmi_plume.h DESTINATION include COMPONENT sedflux)


install (FILES plume.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)

