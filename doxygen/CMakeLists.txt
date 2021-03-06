if (DOT)
  set(HAVE_DOT YES)
else (DOT)
  set(HAVE_DOT NO)
endif (DOT)

set(DOXYGEN_OUTPUT_LANGUAGE "English" CACHE STRING "Language used by doxygen")
set(DOXYGEN_LOG_FILE doxygen.log)
set(DOXYGEN_FULL_PATH_NAMES NO)
set(DOXYGEN_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(DOXYGEN_IMAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/html)
set(DOXYGEN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share)
set(DOXYGEN_SOURCE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/avulsion
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/bing
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/bio
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/compact
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/diffusion
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/failure
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/flow
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/hydrotrend
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/inflow
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/muds
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/plume
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/quake
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/sakura
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/sed
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/sedflux
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/sedutils
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/squall
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/subside
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/../ew/xshore)
string(REGEX REPLACE ";" " " DOXYGEN_INPUT_LIST "${DOXYGEN_SOURCE_DIR}")

CONFIGURE_FILE(sedflux-doxyfile.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/sedflux-doxyfile )

#set( HTML_TARGET ${DOXYGEN_OUTPUT_DIR}/index.html )
#set( HTML_TARGET ${CMAKE_CURRENT_BINARY_DIR}/html-build.stamp )
#ADD_CUSTOM_TARGET( HTML_TARGET ALL)
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html-build.stamp
                   COMMAND ${DOXYGEN} sedflux-doxyfile > /dev/null
                   COMMAND touch html-build.stamp
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sedflux-doxyfile.in.cmake
                   COMMENT "Building API documentation (see ${DOXYGEN_LOG_FILE} for errors/warnings)")
ADD_CUSTOM_TARGET(docs DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html-build.stamp)
                   
                   #DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sedflux-doxyfile.in.cmake
                   #DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sedflux-doxyfile )
#ADD_CUSTOM_TARGET( docs DEPENDS ${HTML_TARGET} )

install( DIRECTORY ${DOXYGEN_OUTPUT_DIR}
         DESTINATION ${DOXYGEN_INSTALL_DIR}
         COMPONENT sedflux)

########### install files ###############




#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = 1.6
#
#TARGET_DIR=$(HTML_DIR)
#
#RUN_DOXYGEN      = @DOXYGEN@
#MTOC             = ./matlab2c
##FLEX             = flex
##GCC              = gcc
#
#DOC_FILES        = \
#                   input_files.txt \
#                   downloading.txt \
#                   installation.txt \
#                   main_page.txt \
#                   programming.txt \
#                   running.txt \
#                   modules.txt \
#                   visualizing.txt
#
#IMAGE_FILES      = \
#                   adriatic-x-section-mask.jpg \
#                   adriatic-x-section-mask.eps \
#                   adriatic-x-section-time.jpg \
#                   adriatic-x-section-time.eps \
#                   adriatic-x-section.jpg \
#                   adriatic-x-section.eps \
#                   elevation-3d.jpg \
#                   elevation-3d.eps \
#                   x-section-3d-property.eps \
#                   x-section-3d-property.jpg \
#                   x-section.eps \
#                   x-section.jpg \
#                   bioturbation_core.eps \
#                   bioturbation_core.jpg \
#                   eel-bioturbation-off.jpg \
#                   eel-bioturbation-off.eps \
#                   eel-bioturbation-diffusion.jpg \
#                   eel-bioturbation-diffusion.eps \
#                   eel-bioturbation-conveyor.jpg \
#                   eel-bioturbation-conveyor.eps \
#                   eel-core-bioturbation-off.jpg \
#                   eel-core-bioturbation-off.eps \
#                   eel-core-bioturbation-diffusion.jpg \
#                   eel-core-bioturbation-diffusion.eps \
#                   eel-core-bioturbation-conveyor.jpg \
#                   eel-core-bioturbation-conveyor.eps
#
#MATLAB_FILES     = \
#                   sedflux/mfiles/plot_property.m \
#                   sedflux/mfiles/read_property.m \
#                   sedflux/mfiles/read_property_header.m \
#                   sedflux/mfiles/plot_measuring_station_surface.m \
#                   sedflux/mfiles/read_measuring_station_cube.m \
#                   sedflux/mfiles/read_tripod_header.m \
#                   sedflux/mfiles/film_measuring_station_surface.m \
#                   sedflux/mfiles/get_property_full_name.m \
#                   sedflux/mfiles/parse_sedflux_header.m \
#                   sedflux/mfiles/parse_varargin.m \
#                   sedflux/mfiles/plot_2d_sedflux_image.m
#
#EXTRA_DIST       = \
#                   $(DOC_FILES) \
#                   $(IMAGE_FILES) \
#                   mtoc.l \
#                   sedflux-doxyfile.in
#
#pkgdata_DATA     = 
#
#DOC_STAMPS=html-build.stamp
#CLEANFILES=$(DOC_STAMPS)
#DISTCLEANFILES   = __main_page.doc \
#                   __matlab_docs.c \
#                   matlab2c
#
#if ENABLE_DOXYGEN
#
#if ENABLE_FLEX
#all-local: matlab-docs html-build.stamp
#else
#all-local: html-build.stamp
#endif
#
#else
#all-local:
#endif
#
#if ENABLE_FLEX
#docs: matlab-docs html-build.stamp
#else
#docs: html-build.stamp
#endif
#
#html-build.stamp:
#	@echo "Building HTML Documentation..."
#	@-chmod -R u+w $(srcdir)
#	rm -rf $(srcdir)/html
#	mkdir $(srcdir)/html
#	if test -f $(srcdir)/__main_page.doc ; then \
#	   rm -f $(srcdir)/__main_page.doc ; \
#	fi
#	touch $(srcdir)/__main_page.doc
#	@for f in $(DOC_FILES) ; do \
#	   cat $(srcdir)/$$f >> $(srcdir)/__main_page.doc ; \
#	done
#	@${RUN_DOXYGEN} sedflux-doxyfile > /dev/null
#	rm -r $(srcdir)/__main_page.doc
#	touch html-build.stamp
#
#matlab-docs: matlab2c __matlab_docs.c
#	@echo "/*@{*/" >> $(srcdir)/__matlab_docs.c ;
#	@for f in $(MATLAB_FILES) ; do \
#	   echo 'Appending ' $(srcdir)/__matlab_docs.c ' with ' $$f ; \
#	   if test -f $(top_srcdir)/$$f ; then \
#	      $(MTOC) $(top_srcdir)/$$f >> $(srcdir)/__matlab_docs.c ; \
#	   else \
#	      echo 'Could not find file: ' $$f ; \
#	   fi ; \
#	done ;
#	@echo "/*@}*/" >> $(srcdir)/__matlab_docs.c ;
#
#__matlab_docs.c: force_rebuild
#	@if test -f $(srcdir)/__matlab_docs.c ; then \
#	   echo 'Removing ' $(srcdir)/__matlab_docs.c ; \
#	   rm -f $(srcdir)/__matlab_docs.c ; \
#	fi
#	@echo "/**" > $(srcdir)/__matlab_docs.c
#	@echo "\defgroup sedflux_mfiles_group Sedflux Matlab Files" >> $(srcdir)/__matlab_docs.c
#	@echo "\image html x-section.jpg \"A jpeg image\"" >> $(srcdir)/__matlab_docs.c
#	@echo "\image latex x-section.eps \"An eps image\"" >> $(srcdir)/__matlab_docs.c
#	@echo "*/" >> $(srcdir)/__matlab_docs.c
#
#
#matlab2c: mtoc.l
#	$(FLEX) $(srcdir)/mtoc.l
#	$(CC) -o matlab2c lex.yy.c -lfl
#	rm -f lex.yy.c
#
#install-data-local:
#	installfiles=`echo $(srcdir)/html/*`; \
#	if test "$$installfiles" = '$(srcdir)/html/*'; then \
#	   echo '--- Nothing to install' ; \
#	else \
#	   $(mkinstalldirs) $(DESTDIR)$(TARGET_DIR); \
#	   for i in $$installfiles; do \
#	      echo '--- Installing '$$i ; \
#	      $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
#	   done; \
#	fi
#
#uninstall-local:
#	rm -f $(DESTDIR)$(TARGET_DIR)/*
#
#dist-hook:
#	mkdir $(distdir)/html
#	-cp $(srcdir)/html/* $(distdir)/html
#
#force_rebuild:
#
