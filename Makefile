# top-level Makefile

# all=clash_tun clash_update
all=$(patsubst %/,%,$(wildcard clash_*/))

.PHONY:$(all)

default_top:$(all)

$(all):
	$(MAKE) --directory=$@

CC:=gcc

CFLAGS:=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -D_GNU_SOURCE -pthread # -fmax-errors=1

%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

clean:
	@find . -type f -a \( -name "*.o" -o -name "*.out" \) -exec rm -v {} \;
