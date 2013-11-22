include_directories(${CMAKE_SOURCE_DIR}/ew )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/avulsion.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/avulsion.pc )

########### next target ###############

set (avulsion_SRCS avulsion_main.c)

add_executable (run_avulsion ${avulsion_SRCS})

target_link_libraries (run_avulsion avulsion-static sedflux-static)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_avulsion
       DESTINATION bin RENAME avulsion COMPONENT sedflux)


########### next target ###############

set (avulsion_LIB_SRCS avulsion.c avulsion_api.c)

set_source_files_properties(${avulsion_LIB_SRCS} get_config_text.c PROPERTIES LANGUAGE CXX)
add_library(avulsion SHARED ${avulsion_LIB_SRCS} get_config_text.c)
add_library(avulsion-static STATIC ${avulsion_LIB_SRCS})

target_link_libraries(avulsion sedflux)

install(TARGETS avulsion DESTINATION lib COMPONENT sedflux)


########### install files ###############

install (FILES avulsion.h avulsion_api.h
         DESTINATION include/ew-2.0
         COMPONENT sedflux)

install (FILES avulsion_api.h
         RENAME bmi_avulsion.h
         DESTINATION include
         COMPONENT sedflux)

install (FILES avulsion.pc DESTINATION lib/pkgconfig  COMPONENT sedflux)

