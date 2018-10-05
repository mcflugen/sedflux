include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. )

include (FindSubversion OPTIONAL RESULT_VARIABLE SUBVERSION)

########### Get subversion revision number ###############

set (DATADIR ${CMAKE_INSTALL_PREFIX}/share)
set (PLUGINDIR ${CMAKE_INSTALL_PREFIX}/lib)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/datadir_path.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/datadir_path.h )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/sed.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sed.pc )

########### next target ###############

SET(sedflux_LIB_SRCS
   csdms.c
   sed_cell.c
   sed_column.c
   sed_cube.c
   sed_diag.c
   sed_epoch.c
   sed_hydro.c
   sed_hydrotrend.c
   sed_process.c
   sed_property.c
   sed_property_file.c
   sed_river.c
   sed_sediment.c
   sed_signal.c
   sed_tripod.c
   sed_wave.c
   sed_input_files.c
)

set_source_files_properties(${sedflux_LIB_SRCS} PROPERTIES LANGUAGE C)

add_library(sedflux SHARED ${sedflux_LIB_SRCS})
add_library(sedflux-static STATIC ${sedflux_LIB_SRCS})

target_link_libraries(sedflux m ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} utils)
target_link_libraries(
  sedflux-static m utils-static
  ${GLIB2_LIBRARIES}
  ${GTHREAD2_LIBRARIES}
)

install(TARGETS sedflux DESTINATION lib COMPONENT sedflux)

########### next target ###############

SET(write_test_file_SRCS
   sed_column_write_test_file.c
)

add_executable(write_test_file ${write_test_file_SRCS})

target_link_libraries(write_test_file sedflux-static gthread-2.0 glib-2.0)

install(TARGETS write_test_file DESTINATION bin COMPONENT sedflux)

########### Optional unit tests ###############

if (WITH_CHECK)
  set(libsed_unit_test_SRCS
    unit_test.c
    sed_cell_unit_test.c
    sed_wave_unit_test.c
    sed_column_unit_test.c
    sed_cube_unit_test.c
    sed_river_unit_test.c
    sed_sediment_unit_test.c
    sed_hydro_unit_test.c
  )

  add_executable( libsed_unit_test ${libsed_unit_test_SRCS} )
  target_link_libraries( libsed_unit_test sedflux-static check )
endif (WITH_CHECK)

set (cell_tests_SRCS test_cell.c test_sed.c)
add_executable (sed-test-cell ${cell_tests_SRCS})
target_link_libraries (sed-test-cell sedflux-static)

set (column_tests_SRCS test_column.c test_sed.c)
add_executable (sed-test-column ${column_tests_SRCS})
target_link_libraries (sed-test-column sedflux-static)

set (cube_tests_SRCS test_cube.c test_sed.c)
add_executable (sed-test-cube ${cube_tests_SRCS})
target_link_libraries (sed-test-cube sedflux-static)

set (hydro_tests_SRCS test_hydro.c test_sed.c)
add_executable (sed-test-hydro ${hydro_tests_SRCS})
target_link_libraries (sed-test-hydro sedflux-static)

set (river_tests_SRCS test_river.c test_sed.c)
add_executable (sed-test-river ${river_tests_SRCS})
target_link_libraries (sed-test-river sedflux-static)

set (sediment_tests_SRCS test_sediment.c test_sed.c)
add_executable (sed-test-sediment ${sediment_tests_SRCS})
target_link_libraries (sed-test-sediment m sedflux-static m)

set (wave_tests_SRCS test_wave.c test_sed.c)
add_executable (sed-test-wave ${wave_tests_SRCS})
target_link_libraries (sed-test-wave sedflux-static)

########### utility programs ###############

set (print_input_file_SRCS print_input.c sed_input_files.c)
add_executable (sed-print-input ${print_input_file_SRCS})
target_link_libraries (sed-print-input sedflux-static gmodule-2.0)
install(TARGETS sed-print-input DESTINATION bin COMPONENT sedflux)

########### install files ###############

install(
  FILES sed.h
  DESTINATION include/ew-2.0
  COMPONENT sedflux
)
install(
  FILES
    csdms.h
    sed_cell.h
    sed_column.h
    sed_const.h
    sed_cube.h
    sed_diag.h
    sed_epoch.h
    sed_hydro.h
    sed_hydrotrend.h
    sed_process.h
    sed_property.h
    sed_property_file.h
    sed_river.h
    sed_sediment.h
    sed_signal.h
    sed_tripod.h
    sed_wave.h
    etk_addrem.h
    etk_keyvalue.h
    datadir_path.h
  DESTINATION include/ew-2.0/sed
  COMPONENT sedflux
)
install(FILES sed.pc DESTINATION lib/pkgconfig COMPONENT sedflux)
