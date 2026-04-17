.PHONY: all clean cleanall

SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:.cc=.o)

PROGRAM = test

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -rdynamic $(OBJECTS) -o $@

run_example: $(PROGRAM)
	@echo "== Goose == "
	@./$(PROGRAM)

clean:
	@rm -rf $(PROGRAM)

cleanall: clean
	@rm -rf $(OBJECTS)
