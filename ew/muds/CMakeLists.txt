include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(muds_SRCS muds_main.c)

add_executable(run_muds ${muds_SRCS})

target_link_libraries(run_muds muds-static sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_muds
  DESTINATION bin
  RENAME muds
  COMPONENT sedflux
)

########### next target ###############

set (muds_LIB_SRCS muds.c)

set_source_files_properties (${muds_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(muds ${muds_LIB_SRCS})
add_library(muds-static STATIC ${muds_LIB_SRCS})

target_link_libraries(muds sedflux)

install(TARGETS muds DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  muds.h DESTINATION include/ew-2.0 COMPONENT sedflux)
