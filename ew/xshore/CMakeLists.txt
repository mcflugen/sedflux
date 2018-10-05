include_directories(${CMAKE_SOURCE_DIR}/ew)

########### next target ###############

set(xshore_LIB_SRCS xshore.c)
set_source_files_properties (${xshore_LIB_SRCS} PROPERTIES LANGUAGE C)
add_library(xshore ${xshore_LIB_SRCS})
add_library(xshore-static STATIC ${xshore_LIB_SRCS})
target_link_libraries(xshore sedflux)
install(
  TARGETS xshore
  DESTINATION lib
  COMPONENT sedflux
)

########### install files ###############

install(FILES xshore.h DESTINATION include/ew-2.0 COMPONENT sedflux)
