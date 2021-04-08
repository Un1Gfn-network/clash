# clash_update Makefile

ifneq ($(CURDIR),/home/darren/clash/clash_misc)
$(error )
endif

include ../Makefile.defs

BIN:=/home/darren/.clash/bin

.PHONY:install_convert install_sh clean

default:
	@$(MAKE) install_sh
	@$(MAKE) install_convert

# "make install_sh" instead of "make /home/darren/.clash/bin/clash.sh" for CLI invocation
install_sh: $(BIN)/clash.sh

$(BIN)/clash.sh: clash.sh
	shellcheck -x $^ || /bin/true
	@echo
	@cp -vf clash.sh $(BIN)/clash.sh
	@:; \
		cd $(BIN); \
		ln -sfv clash.sh clash_run; \
		ln -sfv clash.sh clash_update
	@echo

# "make install_convert" instead of "make /home/darren/.clash/bin/convert.out" for CLI invocation
install_convert: $(BIN)/convert.out

$(BIN)/convert.out: flag.c flag.h convert.c ../restful_port.h
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(BIN)/convert.out $(filter %.c,$^) $(shell pkg-config --libs yaml-0.1 glib-2.0)
	@echo
	@realpath --canonicalize-existing $(BIN)/convert.out
	@echo

uninstall:
	$(MAKE) clean
	@rm -fv $(BIN)/convert.out
	@rm -fv $(BIN)/clash_run
	@rm -fv $(BIN)/clash_update

clean:
	@rm -fv *.out

utf8count.out: utf8count.c; $(CC) $(CFLAGS) -o $@ $<
