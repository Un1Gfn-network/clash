# clash_tun Makefile

ifneq ($(CURDIR),/home/darren/clash/clash_tun)
$(error )
endif

include ../Makefile.defs

default: clash_tun.out

clash_tun.out: \
	bus.o \
	file.o \
	ioctl.o \
	main.o \
	netlink.o \
	privilege.o \
	proc.o \
	profile.o \
	resolv.o \
	restful.o \
	shadowsocks.o \
	status.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread $(shell curl-config --libs) $(shell pkg-config --libs json-c yaml-0.1 shadowsocks-libev libbsd libprocps libsystemd)
	@echo
	@sudo rm -fv /usr/local/bin/$@
	@sudo install -groot -m755 -oroot -v $@ /usr/local/bin/$@
	@echo

bus.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libsystemd)
ioctl.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libbsd)
proc.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libprocps)
profile.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1 json-c shadowsocks-libev)
restful.o: CFLAGS_EXTRA:=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)
shadowsocks.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags shadowsocks-libev)

clean:
	@rm -fv *.o *.out
	@if [ -e $(DEST) ]; then sudo rm -fv $(DEST); else /bin/true; fi

###

# sdbusout::=$(addsuffix .out,$(addprefix bus,-service -client))
# $(MAKE) $(word 1,$(sdbusout))

# call:
# # 	@echo sleep 1; sleep 1
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
# 	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
# 	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
# 	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data

# service:
# 	@killall -SIGINT bus_calc_service.out || /bin/true
# 	@$(MAKE) clean
# 	$(MAKE) bus_calc_service.out
# # 	@./bus_calc_service.out
# # 	@valgrind ./bus_calc_service.out
# 	@valgrind -s --leak-check=full --show-leak-kinds=all ./bus_calc_service.out

# bus_calc_service.out:
# bus_%.out: bus_%.c
# 	$(CC) $(CFLAGS) $(shell pkg-config --cflags libsystemd) -o $@ $< $(shell pkg-config --libs libsystemd)

###

# add:
# 	$(MAKE) clean
# 	$(MAKE) add-server.out add-client.out

# add-%.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags dbus-1)
# add-%.out: LIBS:=$(shell pkg-config --libs dbus-1)

# add-%.out: add-%.o
# 	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

###
