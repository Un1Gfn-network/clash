# top-level Makefile

ifndef TOP_INCLUDED
TOP_INCLUDED = 1
# $(info TOP_INCLUDED=$(TOP_INCLUDED))

SUBDIRS=$(patsubst %/,%,$(wildcard clash_*/))
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables
# MAKEFLAGS += --jobs=4
# test:
# 	@echo MAKEFLAGS=$(MAKEFLAGS)
# 	@echo MAKEOVERRIDES=$(MAKEOVERRIDES)

CC:=gcc
CFLAGS:=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -D_GNU_SOURCE -pthread # -fmax-errors=1
%.o: %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_EXTRA) -o $@ $<

###

.PHONY:$(SUBDIRS) default_top clean_top $(CLEAN)

default_top:$(SUBDIRS)
$(SUBDIRS):
	$(MAKE) --directory=$@

CLEAN=$(addprefix clean_,$(SUBDIRS))
clean_top:$(CLEAN)
$(CLEAN):
	$(MAKE) -C $(patsubst clean_%,%,$@) clean

else
endif
