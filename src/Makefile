###

default:
	$(MAKE) clean
	$(MAKE) clash_tun.out

###

clean:
	@rm -fv *.out *.o

CC::=gcc

CFLAGS::=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -D_GNU_SOURCE -pthread # -fmax-errors=1

%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

###

main.o: CFLAGS_EXTRA:=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)
proc.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libprocps)
conf.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1 json-c)
ioctl.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libbsd)
shadowsocks.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags shadowsocks-libev)

clash_tun.out:main.o conf.o resolv.o shadowsocks.o curl.o netlink.o privilege.o ioctl.o proc.o
	$(CC) $(CFLAGS) -o $@ $^ -pthread $(shell curl-config --libs) $(shell pkg-config --libs json-c yaml-0.1 shadowsocks-libev libbsd libprocps)

###

# sdbusout::=$(addsuffix .out,$(addprefix bus,-service -client))
# $(MAKE) $(word 1,$(sdbusout))

call:
# 	@echo sleep 1; sleep 1
	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data
	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Multiply xx  5 7
	@busctl --user call         net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Divide   xx 72 2
	@busctl --user get-property net.poettering.Calculator /net/poettering/Calculator net.poettering.Calculator Data

service:
	@killall -SIGINT bus_calc_service.out || /bin/true
	@$(MAKE) clean
	$(MAKE) bus_calc_service.out
# 	@./bus_calc_service.out
# 	@valgrind ./bus_calc_service.out
	@valgrind -s --leak-check=full --show-leak-kinds=all ./bus_calc_service.out

bus_calc_service.out:
bus_%.out: bus_%.c
	$(CC) $(CFLAGS) $(shell pkg-config --cflags libsystemd) -o $@ $< $(shell pkg-config --libs libsystemd)

###

CONVERT::=/home/darren/.clash/bin/convert.out
convert:convert.c
	@rm -fv $(CONVERT)
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(CONVERT) $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	ls -lh $(CONVERT)

###

# add:
# 	$(MAKE) clean
# 	$(MAKE) add-server.out add-client.out

# add-%.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags dbus-1)
# add-%.out: LIBS:=$(shell pkg-config --libs dbus-1)

# add-%.out: add-%.o
# 	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

###
