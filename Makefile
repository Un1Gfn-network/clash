###

clean:
	@rm -fv *.out *.o

###

CC::=gcc

CFLAGS::=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline

###

clash_tun.o: CFLAGS_EXTRA:=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)
write_json.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1 json-c)
clash_tun.out: LIBS:=$(shell curl-config --libs) $(shell pkg-config --libs json-c yaml-0.1)

clash_tun.out:clash_tun.o write_json.o resolv.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

###

CONVERT::=/home/darren/.clash/bin/convert.out

convert:convert.c
	@rm -fv $(CONVERT)
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(CONVERT) $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	ls -lh $(CONVERT)

###

route_ioctl.out: CFLAGS_EXTRA:=$(shell pkg-config --cflags json-c yaml-0.1)
route_ioctl.out: LIBS:=$(shell pkg-config --libs json-c yaml-0.1)
route_ioctl.out:route_ioctl.c
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $< $(LIBS)

###

route.out:route.c
	$(CC) $(CFLAGS) -o $@ $<

test.out:test.c
	$(CC) $(CFLAGS) -o $@ $<
