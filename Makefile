# See LICENSE file for copyright and license details.

# Project configuration
include config.mk

MODULES :=
SRC := sdl2_main.c hidamari.c vga.c field.c

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
	@echo "CFLAGS_D  = $(CFLAGS_D)"
	@echo "LDFLAGS_D = $(LDFLAGS_D)"
	@echo "CC      = $(CC)"

clean:
	@echo "Cleaning"
	@rm -rf $(OBJ)
	@rm -f hidamari
	@rm -f hidamari_d

# Object Build Rules
%.o: %.c config.mk
	@echo "CC [R] $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o_d: %.c config.mk
	@echo "CC [D] $@"
	@$(CC) $(CFLAGS_D) -c -o $@ $<

# Targets
hidamari: $(OBJ)
	@echo "CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

hidamari_d: $(OBJ_D)
	@echo "CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS_D)

.PHONY: all options clean
