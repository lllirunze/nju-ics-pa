NAME = fceux

ROM_PATH  = ./nes
ROMS      = $(wildcard $(ROM_PATH)/rom/*.nes)
ROM_SRC   = $(addprefix $(ROM_PATH)/gen/, $(addsuffix .c, $(notdir $(basename $(ROMS)))))
ROM_HEADER = $(ROM_PATH)/gen/roms.h
FCEUX_SRC = $(shell find -L src/ -name "*.c" -o -name "*.cpp")

INC_PATH  = $(ROM_PATH)/gen/
SRCS      = $(FCEUX_SRC) $(ROM_SRC)
CFLAGS    = -DPSS_STYLE=1 -DFRAMESKIP -D__NO_FILE_SYSTEM__

include $(AM_HOME)/Makefile

$(FCEUX_SRC): $(ROM_SRC) $(ROM_HEADER)
$(ROM_SRC) $(ROM_HEADER): rom
.PHONY: rom
rom:
	@-cd $(ROM_PATH) && python3 build-roms.py

clean: myclean

myclean:
	-rm -rf ./nes/gen
