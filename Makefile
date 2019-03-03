# See LICENSE file for copyright and license details.

# Project configuration
include config.mk

SRC := hidamari.c field.c vga.c ai.c

OBJ := $(patsubst %.c, src/%.o, $(filter %.c, $(SRC)))

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
	@rm -f hidamari_d

# Object Build Rules
%.o: %.c config.mk
	@echo "CC [O] $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Targets
hidamari: src/dev.o $(OBJ)
	@echo "CC $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: all options clean
