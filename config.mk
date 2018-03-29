VERSION := 1.0.0

# System paths
PREFIX := /usr/local
BINPREFIX := $(PREFIX)/bin
INCLUDEPREFIX := $(PREFIX)/include
LIBPREFIX := $(PREFIX)/lib
MANPREFIX := $(PREFIX)/man

# Linking flags
LDFLAGS := -lSDL2 -lSDL2_image

# C Compiler settings
CC := cc
CFLAGS := -O2 -Iinclude -std=c11 -pedantic -Wall -Wextra
CFLAGS += -g
