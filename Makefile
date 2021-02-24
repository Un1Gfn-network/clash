# top-level Makefile

ifneq ($(CURDIR),/home/darren/clash)
$(error )
endif

include Makefile.defs

SUBDIRS=$(patsubst %/,%,$(wildcard clash_*/))
.PHONY:$(SUBDIRS) default clean $(CLEAN)

default:$(SUBDIRS)
$(SUBDIRS):
	@echo
	$(MAKE) --directory=$@

CLEAN=$(addprefix clean_,$(SUBDIRS))
clean:$(CLEAN)
$(CLEAN):
	$(MAKE) -C $(patsubst clean_%,%,$@) clean
