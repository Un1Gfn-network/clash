# clash_update Makefile

ifneq ($(CURDIR),/home/darren/clash/clash_misc)
$(error )
endif

include ../Makefile.defs

BIN:=/home/darren/.clash/bin

.PHONY:install_convert install_sh clean

default:
	@$(MAKE) clean
	@$(MAKE) install_convert
	@$(MAKE) install_sh

install_convert:convert.c ../restful_port.h
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(BIN)/convert.out $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	@realpath --canonicalize-existing $(BIN)/convert.out

install_sh:
	shellcheck -x clash.sh || /bin/true
	@echo
	@cp -v clash.sh $(BIN)/clash.sh
	@echo
	@:; \
		cd $(BIN); \
		ln -sfv clash.sh clash_run; \
		ln -sfv clash.sh clash_update
	@echo

clean:
	@rm -fv $(BIN)/convert.out
	@rm -fv $(BIN)/clash_run
	@rm -fv $(BIN)/clash_update
