dist_bin_SCRIPTS = clash.sh

bin_PROGRAMS = clash_convert # utf8count
clash_convert_SOURCES = convert.c flag.c
clash_convert_CFLAGS = $(LIBYAML_CFLAGS) $(GLIB20_CFLAGS) -DRESTFUL_PORT=$(RESTFUL_PORT)
clash_convert_LDADD = $(LIBYAML_LIBS) $(GLIB20_LIBS)

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
		shellcheck -x clash.sh && \
		ln -sv clash.sh clash_run && \
		ln -sv clash.sh clash_update

uninstall-hook:
	test ! -e $(DESTDIR)$(bindir) || cd $(DESTDIR)$(bindir) && \
		test ! -e clash_run    &&  test ! -L clash_run    || rm -v clash_run    && \
		test ! -e clash_update &&  test ! -L clash_update || rm -v clash_update
