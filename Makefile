CC := g++
SRCDIR := src
BINDIR := bin

MAIN := $(SRCDIR)/main.cc
POP := $(SRCDIR)/popGen.cc

CFLAGS := -std=c++17

INC := -I include/

main:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INC) $(MAIN) $^ -o $(BINDIR)/main

pop:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INC) $(POP) $^ -o $(BINDIR)/pop

all: main

run:
	$(BINDIR)/main ${ARGS}

gen:
	$(BINDIR)/pop ${ARGS}

.PHONY: clean coverage