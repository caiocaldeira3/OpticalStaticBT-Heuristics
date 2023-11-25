CC := g++
SRCDIR := src
BINDIR := bin

MAIN := $(SRCDIR)/main.cc

CFLAGS := -std=c++17

INC := -I include/

main:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INC) $(MAIN) $^ -o $(BINDIR)/main

all: main

run:
	$(BINDIR)/main ${ARGS}

.PHONY: clean coverage