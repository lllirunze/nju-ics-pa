TEST_ISA = i m #a f d c i m
EXCLUDE_TEST = fence_i ma_data

SUPPORTED_AM_ISA = riscv64 riscv32 riscv64e riscv32e riscv32mini
AM_ISA = $(word 1, $(subst -, ,$(ARCH)))

ifeq ($(findstring $(MAKECMDGOALS),clean),)  # ignore the commands if the target is clean
ifeq ($(filter $(SUPPORTED_AM_ISA), $(AM_ISA)), )
  $(error Expected $$ISA in {$(SUPPORTED_AM_ISA)}, Got "$(AM_ISA)")
endif
XLEN = $(shell echo $(AM_ISA) | grep -o '32\|64')
TEST_DIR = $(TEST_ISA:%=isa/rv$(XLEN)u%)
endif

ORIGINAL_TEST = $(basename $(notdir $(shell find $(TEST_DIR) -name "*.S" | sort)))
ALL = $(filter-out $(EXCLUDE_TEST),$(ORIGINAL_TEST))
ALL_SRCS = $(foreach d,$(TEST_DIR),$(foreach f,$(ALL),$(wildcard $(d)/$(f).S)))

RESULT = .result
$(shell > $(RESULT))

COLOR_RED   = \033[1;31m
COLOR_GREEN = \033[1;32m
COLOR_NONE  = \033[0m

define find_src
  $(filter %/$(1).S,$(ALL_SRCS))
endef

all: $(addprefix Makefile-, $(ALL))
	@echo "test list [$(words $(ALL)) item(s)]:" $(ALL)

$(ALL): %: Makefile-%

.SECONDEXPANSION:  # this helps to call function in prerequisite
Makefile-%: $$(call find_src,%)
	@/bin/echo -e "NAME = $*\nSRCS = $<\nINC_PATH += $(shell pwd)/env/p $(shell pwd)/isa/macros/scalar\ninclude $${AM_HOME}/Makefile" > $@
	@if make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS); then \
		printf "[%14s] $(COLOR_GREEN)PASS$(COLOR_NONE)\n" $* >> $(RESULT); \
	else \
		printf "[%14s] $(COLOR_RED)***FAIL***$(COLOR_NONE)\n" $* >> $(RESULT); \
	fi
	-@rm -f Makefile-$*

run: all
	@cat $(RESULT)
	@rm $(RESULT)

gdb: all

clean:
	rm -rf Makefile-* build/

.PHONY: all run clean $(ALL)
