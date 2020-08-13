CC::=gcc -std=gnu11 -Wno-unused-parameter -Wall -Wextra -O0 -g

CFLAGS+=$(shell curl-config --cflags) $(shell pkg-config --cflags json-c)

LDFLAGS+=$(shell curl-config --libs) $(shell pkg-config --libs json-c)

00_restful.out:00_restful.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@rm -fv *.out