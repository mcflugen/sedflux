include_directories(${CMAKE_SOURCE_DIR}/ew )

set(SUBSIDE_VERSION "0.1")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/subside.pc.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/subside.pc
)

########### next target ###############

set(subside_SRCS subside_main.c)
add_executable(run_subside ${subside_SRCS})
target_link_libraries(run_subside bmi_subside subside-static sedflux-static)
install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_subside
  DESTINATION bin
  RENAME subside
  COMPONENT sedflux
)

########### next target ###############

set(subside_LIB_SRCS subside.c subside_api.c)
set_source_files_properties(
  ${subside_LIB_SRCS}
  get_config_text.c
  PROPERTIES LANGUAGE C
)
add_library(subside ${subside_LIB_SRCS} get_config_text.c)
add_library(subside-static STATIC ${subside_LIB_SRCS})
target_link_libraries(subside sedflux)
install(TARGETS subside DESTINATION lib COMPONENT sedflux)

########### install files ###############

install(
  FILES subside.h subside_api.h
  DESTINATION include/ew-2.0
  COMPONENT sedflux
)
install(FILES subside.pc DESTINATION lib/pkgconfig COMPONENT sedflux)

### BMI

set(bmi_lib_sources bmi_subside.c)
add_library(bmi_subside ${bmi_lib_sources})
add_library(bmi_subside-static STATIC ${bmi_lib_sources})
target_link_libraries(bmi_subside subside sedflux)
target_link_libraries(bmi_subside-static subside-static subside-static)
set_target_properties(bmi_subside-static PROPERTIES OUTPUT_NAME bmi_subside)

install(
  TARGETS bmi_subside
  DESTINATION lib
)
install(
  TARGETS bmi_subside-static
  DESTINATION lib
)
install(
  FILES bmi_subside.h
  DESTINATION include/subside
)
install(
  FILES bmi.h
  DESTINATION include/subside
)
