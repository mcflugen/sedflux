include_directories(${CMAKE_SOURCE_DIR}/ew )

########### next target ###############

set(squall_SRCS squall_main.c)
add_executable(run_squall ${squall_SRCS})
target_link_libraries(run_squall squall-static sedflux-static)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_squall DESTINATION bin RENAME squall COMPONENT sedflux)

########### next target ###############

set(squall_LIB_SRCS squall.c)
set_source_files_properties(
  ${squall_LIB_SRCS}
  get_config_text.c
  PROPERTIES LANGUAGE C
)
add_library(squall ${squall_LIB_SRCS} get_config_text.c)
add_library(squall-static STATIC ${squall_LIB_SRCS})
target_link_libraries(squall sedflux)
install(TARGETS squall DESTINATION lib COMPONENT sedflux)

########### install files ###############

install(FILES squall.h DESTINATION include/ew-2.0 COMPONENT sedflux)
