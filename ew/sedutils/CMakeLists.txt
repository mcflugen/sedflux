include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

set(read_hydro_SRCS read_hydro.c)
add_executable(read_hydro ${read_hydro_SRCS})
target_link_libraries(read_hydro sedflux)
install(TARGETS read_hydro DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedrescale_SRCS sedrescale.c)
add_executable(sedrescale ${sedrescale_SRCS})
target_link_libraries(sedrescale sedflux)
install(TARGETS sedrescale DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(getfloor_SRCS getfloor.c)
add_executable(getfloor ${getfloor_SRCS})
target_link_libraries(getfloor sedflux)
install(TARGETS getfloor DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedextract_SRCS sedextract.c)
add_executable(sedextract ${sedextract_SRCS})
target_link_libraries(sedextract sedflux)
install(TARGETS sedextract DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sed2binary_SRCS sed2binary.c)
add_executable(sed2binary ${sed2binary_SRCS})
target_link_libraries(sed2binary sedflux)
install(TARGETS sed2binary DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(read_usgs_SRCS read_usgs.c)
add_executable(read_usgs ${read_usgs_SRCS})
target_link_libraries(read_usgs sedflux)
install(TARGETS read_usgs DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(create_hydro_SRCS create_hydro.c)
add_executable(create_hydro ${create_hydro_SRCS})
target_link_libraries(create_hydro sedflux)
install(TARGETS create_hydro DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedwheeler_SRCS sedwheeler.c)
add_executable(sedwheeler ${sedwheeler_SRCS})
target_link_libraries(sedwheeler sedflux)
install(TARGETS sedwheeler DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedflux-make-bathy_SRCS sedflux-make-bathy.c )
add_executable(sedflux-make-bathy ${sedflux-make-bathy_SRCS})
target_link_libraries(sedflux-make-bathy sedflux)
install(TARGETS sedflux-make-bathy DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedflux-read-hydro_SRCS sedflux-read-hydro.c)
add_executable(sedflux-read-hydro ${sedflux-read-hydro_SRCS})
target_link_libraries(sedflux-read-hydro sedflux)
install(TARGETS sedflux-read-hydro DESTINATION bin COMPONENT sedflux)

########### next target ###############

set(sedflux-make-sequence_SRCS sedflux-make-sequence.c)
add_executable(sedflux-make-sequence ${sedflux-make-sequence_SRCS})
target_link_libraries(sedflux-make-sequence sedflux)
install(TARGETS sedflux-make-sequence DESTINATION bin COMPONENT sedflux)
