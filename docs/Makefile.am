AUTOMAKE_OPTIONS = 1.6

TARGET_DIR=$(HTML_DIR)

RUN_DOXYGEN      = @DOXYGEN@

DOC_FILES        = \
                   input_files.txt \
                   downloading.txt \
                   installation.txt \
                   main_page.txt \
                   programming.txt \
                   running.txt

EXTRA_DIST       = \
                   $(DOC_FILES) \
                   sedflux-doxyfile.in
pkgdata_DATA     = 

DOC_STAMPS=html-build.stamp
CLEANFILES=$(DOC_STAMPS)
DISTCLEANFILES=__main_page.doc

if ENABLE_DOXYGEN
all-local: html-build.stamp
else
all-local:
endif

docs: html-build.stamp

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


