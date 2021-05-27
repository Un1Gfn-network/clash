# https://stackoverflow.com/a/42479100

# CLEANFILES = doxyfile.stamp
# doxyfile.stamp: Doxyfile

# all-local: doxyfile.stamp
all-local: html/

html/: Doxyfile
	rm -rf $@
	$(DOXYGEN) $^
	@test -f html/index.html
	@echo
	@printf "  " && realpath html/index.html
	@echo

clean-local:
# 	rm -rf $(top_builddir)/doc/html/
	rm -rf html/

# install-exec-am
# install-data-am
