# clash_update Makefile

CONVERT:=/home/darren/.clash/bin/convert.out

default:convert.c
	@$(MAKE) clean
	$(CC) $(CFLAGS) $(shell pkg-config --cflags yaml-0.1 glib-2.0) -o $(CONVERT) $< $(shell pkg-config --libs yaml-0.1 glib-2.0)
	@ls -lh $(CONVERT)

clean:
	@rm -fv $(CONVERT)

include ../Makefile
