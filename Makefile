# See LICENSE file for copyright and license details.

# Project configuration
include config.mk

MODULES :=
SRC := sdl2_main.c hidamari.c region.c ai.c vga.c

# Project modules
include $(patsubst %, src/%/module.mk, $(MODULES))

OBJ := $(patsubst %.c, src/%.o, $(filter %.c, $(SRC)))
OBJ_D := $(patsubst %.c, src/%.o_d, $(filter %.c, $(SRC)))

# Standard targets
all: hidamari

options:
	@echo "Build options:"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"

clean:
	@echo "Cleaning"
	@rm -rf $(OBJ)
	@rm -f hidamari

# Object Build Rules
%.o: %.c config.mk
	@echo "CC [R] $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Targets
hidamari: $(OBJ)
	@echo "CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: all options clean
