CC::=gcc

CFLAGS::=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline

# .PRECIOUS: %.o

main.o: CFLAGS_EXTRA:=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)
LIBS+=$(shell curl-config --libs) $(shell pkg-config --libs json-c)

yaml.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1)
LIBS+=$(shell pkg-config --libs yaml-0.1)

extract.o: CFLAGS_EXTRA:=$(shell pkg-config --cflags yaml-0.1 json-c)

# test.out:test.c
# 	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

clash_tun.out:main.o extract.o resolv.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	@rm -fv *.out *.o

CONVERT::=/home/darren/.clash/bin/convert.out

convert:convert.c
	@rm -fv $(CONVERT)
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(CONVERT) $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	ls -lh $(CONVERT)
