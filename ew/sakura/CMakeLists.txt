include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(sakura_SRCS sakura_main.c)

add_executable(run_sakura ${sakura_SRCS})

target_link_libraries(run_sakura sakura-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_sakura DESTINATION bin RENAME sakura COMPONENT sedflux)


########### next target ###############

SET(sakura_LIB_SRCS
  sakura.c
  sakura_io.c
  sakura_sedflux_interface.c
  sakura_utils.c
)

set_source_files_properties(
  ${sakura_LIB_SRCS}
  get_config_text.c
  PROPERTIES LANGUAGE C
)
add_library(sakura ${sakura_LIB_SRCS} get_config_text.c)
add_library(sakura-static STATIC ${sakura_LIB_SRCS})

target_link_libraries(sakura sedflux)

install(TARGETS sakura DESTINATION lib COMPONENT sedflux)


########### install files ###############

install(FILES  sakura.h DESTINATION include/ew-2.0 COMPONENT sedflux)
