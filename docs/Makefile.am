AUTOMAKE_OPTIONS = 1.6

TARGET_DIR=$(datadir)/ew-doc/html

RUN_DOXYGEN      = @DOXYGEN@

EXTRA_DIST       = main_page.txt sedflux-doxyfile.in
pkgdata_DATA     = 

if HAVE_DOXYGEN
#EXTRA_DIST      += ew-doc.tar 
#pkgdata_DATA    += ew-doc.tar
#EXTRA_DIST      +=
#pkgdata_DATA    += html latex

#htmldir = $(prefix)/html

#MOSTLYCLEANFILES = html latex
endif

DOC_STAMPS=html-build.stamp
CLEANFILES=$(DOC_STAMPS)

all-local: html-build.stamp
docs: html-build.stamp

html-build.stamp:
	@echo "Building HTML Documentation..."
	@-chmod -R u+w $(srcdir)
	rm -rf $(srcdir)/html
	mkdir $(srcdir)/html
	@${RUN_DOXYGEN} sedflux-doxyfile > /dev/null
	touch html-build.stamp

ew-doc.tar:
	test -d html-latex-doc || mkdir html-latex-doc
	@echo "Generating documentation..."
	@${RUN_DOXYGEN} sedflux-doxyfile > /dev/null
	@echo "Done"
	tardir="html-latex-doc" && $(am__tar) >$ ew-doc.tar
	rm -r html-latex-doc

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

#html:
#	@echo "Generating documentation..."
#	@${RUN_DOXYGEN} > /dev/null
#	@echo "Done"

#install: html
#	mkdir -p $(docdir) \
#	&& chmod a+w $(docdir) \
#	&& cp -r ./html ${docdir} \
#	&& cp -r ./latex ${docdir} \
#	&& chmod -R a-w $(docdir)

#uninstall:
#	rm -r $(docdir)/html \
#	&& rm -r $(docdir)/latex
#
#force_rebuild:

#clean:
#	rm -r ./html ./latex

#dist-hook:
#	make all
#	cp -r $(top_builddir)/docs/html $(distdir)/docs && \
#	cp -r $(top_builddir)/docs/latex $(distdir)/docs



