include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(compact_SRCS compact_main.c)

add_executable(run_compact ${compact_SRCS})

target_link_libraries(run_compact compact-static sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_compact
  DESTINATION bin
  RENAME compact
  COMPONENT sedflux
)


########### next target ###############

set (compact_LIB_SRCS compact.c)

set_source_files_properties (${compact_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(compact ${compact_LIB_SRCS})
add_library(compact-static STATIC ${compact_LIB_SRCS})

target_link_libraries(compact sedflux)

install(TARGETS compact DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  compact.h DESTINATION include/ew-2.0 COMPONENT sedflux)
