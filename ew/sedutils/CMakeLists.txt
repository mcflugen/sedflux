include_directories(${CMAKE_SOURCE_DIR}/ew )


########### next target ###############

SET(read_hydro_SRCS read_hydro.c)

add_executable(read_hydro ${read_hydro_SRCS})

target_link_libraries(read_hydro sedflux)

install(TARGETS read_hydro DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedrescale_SRCS
   sedrescale.c
)

add_executable(sedrescale ${sedrescale_SRCS})

target_link_libraries(sedrescale sedflux)

install(TARGETS sedrescale DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(getfloor_SRCS
   getfloor.c
)

add_executable(getfloor ${getfloor_SRCS})

target_link_libraries(getfloor sedflux)

install(TARGETS getfloor DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedextract_SRCS
   sedextract.c
)

add_executable(sedextract ${sedextract_SRCS})

target_link_libraries(sedextract sedflux)

install(TARGETS sedextract DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sed2binary_SRCS
   sed2binary.c
)

add_executable(sed2binary ${sed2binary_SRCS})

target_link_libraries(sed2binary sedflux)

install(TARGETS sed2binary DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(read_usgs_SRCS
   read_usgs.c
)

add_executable(read_usgs ${read_usgs_SRCS})

target_link_libraries(read_usgs sedflux)

install(TARGETS read_usgs DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(create_hydro_SRCS
   create_hydro.c
)

add_executable(create_hydro ${create_hydro_SRCS})

target_link_libraries(create_hydro sedflux)

install(TARGETS create_hydro DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedwheeler_SRCS
   sedwheeler.c
)

add_executable(sedwheeler ${sedwheeler_SRCS})

target_link_libraries(sedwheeler sedflux)

install(TARGETS sedwheeler DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedflux-make-bathy_SRCS sedflux-make-bathy.c )

add_executable(sedflux-make-bathy ${sedflux-make-bathy_SRCS})

target_link_libraries(sedflux-make-bathy sedflux)

install(TARGETS sedflux-make-bathy DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedflux-read-hydro_SRCS sedflux-read-hydro.c)

add_executable(sedflux-read-hydro ${sedflux-read-hydro_SRCS})

target_link_libraries(sedflux-read-hydro sedflux)

install(TARGETS sedflux-read-hydro DESTINATION bin COMPONENT sedflux)


########### next target ###############

SET(sedflux-make-sequence_SRCS sedflux-make-sequence.c)

add_executable(sedflux-make-sequence ${sedflux-make-sequence_SRCS})

target_link_libraries(sedflux-make-sequence sedflux)

install(TARGETS sedflux-make-sequence DESTINATION bin COMPONENT sedflux)


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES                 = -I$(top_srcdir)/ew
#
#AM_LDFLAGS               = $(top_builddir)/ew/utils/libutils.la \
#                           $(top_builddir)/ew/sed/libsedflux.la
#
##EXTRA_DIST                = acsed.m4
#
#bin_PROGRAMS              = read_hydro \
#                            sedrescale \
#                            getfloor \
#                            sedextract \
#                            sed2binary \
#                            read_usgs \
#                            create_hydro \
#                            sedwheeler \
#                            sedflux-make-bathy \
#                            sedflux-read-hydro \
#                            sedflux-make-sequence
#
#read_hydro_SOURCES            = read_hydro.c
#sedrescale_SOURCES            = sedrescale.c
#getfloor_SOURCES              = getfloor.c
#sedextract_SOURCES            = sedextract.c
#sed2binary_SOURCES            = sed2binary.c
#read_usgs_SOURCES             = read_usgs.c
#create_hydro_SOURCES          = create_hydro.c
#sedwheeler_SOURCES            = sedwheeler.c
#sedflux_make_bathy_SOURCES    = sedflux-make-bathy.c
#sedflux_make_sequence_SOURCES = sedflux-make-sequence.c
#sedflux_read_hydro_SOURCES    = sedflux-read-hydro.c
#
#LDADD                     = -lglib-2.0 -lutils
#
