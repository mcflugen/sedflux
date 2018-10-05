include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(diffusion_SRCS diffusion_main.c)

add_executable(run_diffusion ${diffusion_SRCS})

target_link_libraries(run_diffusion diffusion-static sedflux-static)

install(
  PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_diffusion
  DESTINATION bin
  RENAME diffusion
  COMPONENT sedflux
)

########### next target ###############

set (diffusion_LIB_SRCS diffusion.c)

set_source_files_properties(
  ${diffusion_LIB_SRCS}
  get_config_text.c
  PROPERTIES LANGUAGE C
)
add_library(diffusion ${diffusion_LIB_SRCS} get_config_text.c)
add_library(diffusion-static STATIC ${diffusion_LIB_SRCS})

target_link_libraries(diffusion sedflux)

install(TARGETS diffusion DESTINATION lib COMPONENT sedflux)

########### install files ###############

install(FILES  diffusion.h DESTINATION include/ew-2.0 COMPONENT sedflux)
