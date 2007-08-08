AUTOMAKE_OPTIONS = 1.6

TARGET_DIR=$(HTML_DIR)

RUN_DOXYGEN      = @DOXYGEN@
MTOC             = ./matlab2c
#FLEX             = flex
#GCC              = gcc

DOC_FILES        = \
                   input_files.txt \
                   downloading.txt \
                   installation.txt \
                   main_page.txt \
                   programming.txt \
                   running.txt \
                   modules.txt \
                   visualizing.txt

IMAGE_FILES      = \
                   adriatic-x-section-mask.jpg \
                   adriatic-x-section-mask.eps \
                   adriatic-x-section-time.jpg \
                   adriatic-x-section-time.eps \
                   adriatic-x-section.jpg \
                   adriatic-x-section.eps \
                   elevation-3d.jpg \
                   elevation-3d.eps \
                   x-section-3d-property.eps \
                   x-section-3d-property.jpg \
                   x-section.eps \
                   x-section.jpg \
                   bioturbation_core.eps \
                   bioturbation_core.jpg \
                   eel-bioturbation-off.jpg \
                   eel-bioturbation-off.eps \
                   eel-bioturbation-diffusion.jpg \
                   eel-bioturbation-diffusion.eps \
                   eel-bioturbation-conveyor.jpg \
                   eel-bioturbation-conveyor.eps \
                   eel-core-bioturbation-off.jpg \
                   eel-core-bioturbation-off.eps \
                   eel-core-bioturbation-diffusion.jpg \
                   eel-core-bioturbation-diffusion.eps \
                   eel-core-bioturbation-conveyor.jpg \
                   eel-core-bioturbation-conveyor.eps

MATLAB_FILES     = \
                   sedflux/mfiles/plot_property.m \
                   sedflux/mfiles/read_property.m \
                   sedflux/mfiles/read_property_header.m \
                   sedflux/mfiles/plot_measuring_station_surface.m \
                   sedflux/mfiles/read_measuring_station_cube.m \
                   sedflux/mfiles/read_tripod_header.m \
                   sedflux/mfiles/film_measuring_station_surface.m \
                   sedflux/mfiles/get_property_full_name.m \
                   sedflux/mfiles/parse_sedflux_header.m \
                   sedflux/mfiles/parse_varargin.m \
                   sedflux/mfiles/plot_2d_sedflux_image.m

EXTRA_DIST       = \
                   $(DOC_FILES) \
                   $(IMAGE_FILES) \
                   mtoc.l \
                   sedflux-doxyfile.in

pkgdata_DATA     = 

DOC_STAMPS=html-build.stamp
CLEANFILES=$(DOC_STAMPS)
DISTCLEANFILES   = __main_page.doc \
                   __matlab_docs.c \
                   matlab2c

if ENABLE_DOXYGEN

if ENABLE_FLEX
all-local: matlab-docs html-build.stamp
else
all-local: html-build.stamp
endif

else
all-local:
endif

if ENABLE_FLEX
docs: matlab-docs html-build.stamp
else
docs: html-build.stamp
endif

html-build.stamp:
	@echo "Building HTML Documentation..."
	@-chmod -R u+w $(srcdir)
	rm -rf $(srcdir)/html
	mkdir $(srcdir)/html
	if test -f $(srcdir)/__main_page.doc ; then \
	   rm -f $(srcdir)/__main_page.doc ; \
	fi
	touch $(srcdir)/__main_page.doc
	@for f in $(DOC_FILES) ; do \
	   cat $(srcdir)/$$f >> $(srcdir)/__main_page.doc ; \
	done
	@${RUN_DOXYGEN} sedflux-doxyfile > /dev/null
	rm -r $(srcdir)/__main_page.doc
	touch html-build.stamp

matlab-docs: matlab2c __matlab_docs.c
	@echo "/*@{*/" >> $(srcdir)/__matlab_docs.c ;
	@for f in $(MATLAB_FILES) ; do \
	   echo 'Appending ' $(srcdir)/__matlab_docs.c ' with ' $$f ; \
	   if test -f $(top_srcdir)/$$f ; then \
	      $(MTOC) $(top_srcdir)/$$f >> $(srcdir)/__matlab_docs.c ; \
	   else \
	      echo 'Could not find file: ' $$f ; \
	   fi ; \
	done ;
	@echo "/*@}*/" >> $(srcdir)/__matlab_docs.c ;

__matlab_docs.c: force_rebuild
	@if test -f $(srcdir)/__matlab_docs.c ; then \
	   echo 'Removing ' $(srcdir)/__matlab_docs.c ; \
	   rm -f $(srcdir)/__matlab_docs.c ; \
	fi
	@echo "/**" > $(srcdir)/__matlab_docs.c
	@echo "\defgroup sedflux_mfiles_group Sedflux Matlab Files" >> $(srcdir)/__matlab_docs.c
	@echo "\image html x-section.jpg \"A jpeg image\"" >> $(srcdir)/__matlab_docs.c
	@echo "\image latex x-section.eps \"An eps image\"" >> $(srcdir)/__matlab_docs.c
	@echo "*/" >> $(srcdir)/__matlab_docs.c


matlab2c: mtoc.l
	$(FLEX) $(srcdir)/mtoc.l
	$(CC) -o matlab2c lex.yy.c -lfl
	rm -f lex.yy.c

install-data-local:
	installfiles=`echo $(srcdir)/html/*`; \
	if test "$$installfiles" = '$(srcdir)/html/*'; then \
	   echo '--- Nothing to install' ; \
	else \
	   $(mkinstalldirs) $(DESTDIR)$(TARGET_DIR); \
	   for i in $$installfiles; do \
	      echo '--- Installing '$$i ; \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	   done; \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(TARGET_DIR)/*

dist-hook:
	mkdir $(distdir)/html
	-cp $(srcdir)/html/* $(distdir)/html

force_rebuild:

