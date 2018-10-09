include_directories(${CMAKE_SOURCE_DIR}/ew )

########### next target ###############

set (earthquake_SRCS earthquake_main.c)

add_executable(run_earthquake ${earthquake_SRCS})

target_link_libraries(run_earthquake quake-static sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_earthquake
  DESTINATION bin
  RENAME earthquake
  COMPONENT sedflux
)

########### next target ###############

set(quake_LIB_SRCS earthquake.c)

set_source_files_properties(${quake_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(quake ${quake_LIB_SRCS})
add_library(quake-static STATIC ${quake_LIB_SRCS})

target_link_libraries(quake utils)

install(TARGETS quake DESTINATION lib COMPONENT sedflux)

########### install files ###############

install(FILES earthquake.h DESTINATION include/ew-2.0 COMPONENT sedflux)
