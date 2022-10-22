VERSION := 1.0.0

# System paths
PREFIX := /usr/local
BINPREFIX := $(PREFIX)/bin
INCLUDEPREFIX := $(PREFIX)/include
LIBPREFIX := $(PREFIX)/lib
MANPREFIX := $(PREFIX)/man

# Linking flags
LDFLAGS := -lSDL2 -lSDL2_image -lpthread -lm

# C Compiler settings
CC := cc
CFLAGS := -O2 -I. -std=gnu11 -pedantic -Wall -Wextra
CFLAGS += -O0 -g
