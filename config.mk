VERSION := 0.1.0

# System paths
PREFIX := /usr/local
BINPREFIX := $(PREFIX)/bin
INCLUDEPREFIX := $(PREFIX)/include
LIBPREFIX := $(PREFIX)/lib
MANPREFIX := $(PREFIX)/man

# Linking flags
LDFLAGS := -lpthread -lm

# C Compiler settings
CC := clang
CFLAGS := -O2 -Iinclude -std=c11 -pedantic -Wall -Wextra -g
