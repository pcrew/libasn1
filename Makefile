
SHELL := bash

TOPDIR = $(shell /bin/pwd)
INCDIR = $(TOPDIR)/include

CC = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE) strip
CPPFLAGS = -Wall -std=gnu++17 -I$(INCDIR) -O3

export CC STRIP CPPFLAGS TOPDIR INCDIR

all: test example

test:
	$(MAKE) -C tests

example:
	$(MAKE) -C examples

clean:
	@rm -vf `find . -name *.o`
	@rm -vf `find . -name *.sw`

cleanall: clean
	@$(MAKE) cleanall -C examples
