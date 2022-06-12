install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
		shellcheck -x clash.sh && \
		ln -sv clash.sh clash_run && \
		ln -sv clash.sh clash_update

uninstall-hook:
	test ! -e $(DESTDIR)$(bindir) || cd $(DESTDIR)$(bindir) && \
		test ! -e clash_run    &&  test ! -L clash_run    || rm -v clash_run    && \
		test ! -e clash_update &&  test ! -L clash_update || rm -v clash_update
