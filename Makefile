
SHELL := bash

TOPDIR = $(shell /bin/pwd)
INCDIR = $(TOPDIR)/include

CC = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE) strip
CPPFLAGS = -Wall -O3 -std=gnu++17 -I$(INCDIR)

export CC STRIP CPPFLAGS TOPDIR INCDIR

all: fixture tests example bench

fixture:
	$(MAKE) -C fixtures

tests:
	$(MAKE) -C tests

example:
	$(MAKE) -C examples

bench:
	$(MAKE) -C bench

clean:
	@$(MAKE) clean -C fixtures
	@$(MAKE) clean -C tests
	@$(MAKE) clean -C examples
	@$(MAKE) clean -C bench

cleanall: clean
	@$(MAKE) cleanall -C fixtures
	@$(MAKE) cleanall -C tests
	@$(MAKE) cleanall -C examples
	@$(MAKE) cleanall -C bench

.PHONY: all fixture tests example bench clean cleanall
