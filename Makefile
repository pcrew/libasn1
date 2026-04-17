
SHELL := bash

TOPDIR = $(shell /bin/pwd)
INCDIR = $(TOPDIR)/include
# Keep in sync with examples/Makefile (each subdir builds ./test).
EXAMPLE_SUBDIRS = ldap krb5 snmp goose sv

CC = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE) strip
CPPFLAGS = -Wall -O3 -std=gnu++17 -I$(INCDIR)

export CC STRIP CPPFLAGS TOPDIR INCDIR

PYTHON ?= python3
CLANG_FORMAT ?= clang-format

export CLANG_FORMAT

all: gen-protocols fixture tests example bench

gen-protocols:
	@$(PYTHON) $(TOPDIR)/scripts/gen_protocols.py

fixture: 
	$(MAKE) -C fixtures

tests:
	$(MAKE) -C tests

run_tests: tests
	$(TOPDIR)/tests/test

example: fixture
	$(MAKE) -C examples

examples: example

run_example: examples
	set -e; for d in $(EXAMPLE_SUBDIRS); do \
		echo "== examples/$$d =="; \
		"$(TOPDIR)/examples/$$d/test"; \
	done

bench: fixture
	$(MAKE) -C bench

clean:
	@rm -f -r $(INCDIR)/libasn/protocols/*
	@$(MAKE) clean -C fixtures
	@$(MAKE) clean -C tests
	@$(MAKE) clean -C examples
	@$(MAKE) clean -C bench

cleanall: clean
	@$(MAKE) cleanall -C fixtures
	@$(MAKE) cleanall -C tests
	@$(MAKE) cleanall -C examples
	@$(MAKE) cleanall -C bench

.PHONY: all gen-protocols fixture tests run_tests example examples run_example bench clean cleanall
