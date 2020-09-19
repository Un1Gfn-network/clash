###

all:
	$(MAKE) clean
	$(MAKE) clash_tun.out route.out route_ioctl.out

route:
	$(MAKE) clean
	$(MAKE) route.out route_ioctl.out

clean:
	@rm -fv *.out *.o

CC::=gcc

CFLAGS::=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline # -fmax-errors=1

%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

###

clash_tun.o: CFLAGS_EXTRA:=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)
write_json.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1 json-c)
clash_tun.out: LIBS:=$(shell curl-config --libs) $(shell pkg-config --libs json-c yaml-0.1)
clash_tun.out:clash_tun.o write_json.o resolv.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

###

CONVERT::=/home/darren/.clash/bin/convert.out
convert:convert.c
	@rm -fv $(CONVERT)
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(CONVERT) $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	ls -lh $(CONVERT)

###

jsrv.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags json-c)
route_ioctl.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags libbsd)
route_ioctl.out: LIBS:=$(shell pkg-config --libs json-c libbsd)
route.out: LIBS:=$(shell pkg-config --libs json-c)

route_ioctl.out:def.h jsrv.o route_ioctl.o
	$(CC) $(CFLAGS) -o $@ $(filter-out $<,$^) $(LIBS)

route.out:def.h jsrv.o route.c
	$(CC) $(CFLAGS) -o $@ $(filter-out $<,$^) $(LIBS)

###

# test.out:test.c
# 	$(CC) $(CFLAGS) -o $@ $<

###

add:
	$(MAKE) clean
	$(MAKE) add-server.out add-client.out

add-%.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags dbus-1)
add-%.out: LIBS:=$(shell pkg-config --libs dbus-1)

add-%.out: add-%.o
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
