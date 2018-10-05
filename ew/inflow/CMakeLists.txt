include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(inflow_SRCS inflow_main.c)

add_executable(run_inflow ${inflow_SRCS})

target_link_libraries(run_inflow inflow-static sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_inflow
  DESTINATION bin
  RENAME inflow
  COMPONENT sedflux
)

########### next target ###############

set (pseudoplume_SRCS pseudoplume.c)

add_executable(pseudoplume ${pseudoplume_SRCS})

target_link_libraries(pseudoplume inflow-static sedflux-static)

install(TARGETS pseudoplume DESTINATION bin COMPONENT sedflux)


########### next target ###############

set (inflow_LIB_SRCS inflow.c inflow_io.c inflow_sedflux_interface.c)

set_source_files_properties (${inflow_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(inflow ${inflow_LIB_SRCS})
add_library(inflow-static STATIC ${inflow_LIB_SRCS})

target_link_libraries(inflow sedflux)

install(TARGETS inflow DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  inflow.h DESTINATION include/ew-2.0 COMPONENT sedflux)
