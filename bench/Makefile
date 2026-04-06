.PHONY: all clean cleanall fixture perf-full perf-record-full perf-report

SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:.cc=.o)
PROGRAM = test

FIXTURE := $(abspath ../fixtures/search_result_entry_large.bin)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(FIXTURE)
	$(CC) -rdynamic $(OBJECTS) -o $@ -lbenchmark -pthread

fixture: $(FIXTURE)

$(FIXTURE):
	$(MAKE) -C ../fixtures

PERF = perf

perf-full: $(PROGRAM) $(FIXTURE)
	$(PERF) stat -e cycles,instructions,branch-instructions,branch-misses,cache-references,cache-misses,task-clock,page-faults \
		./$(PROGRAM) --benchmark_filter=BM_libasn_full_decode_all_attrs --benchmark_min_time=2s --benchmark_repetitions=5

perf-record-full: $(PROGRAM) $(FIXTURE)
	@mkdir -p $(dir $(PERF_DATA))
	$(PERF) record -o $(PERF_DATA) -e cycles,branch-misses -b -- \
		./$(PROGRAM) --benchmark_filter=BM_libasn_full_decode_all_attrs --benchmark_min_time=1s

clean:
	@rm -rf $(PROGRAM) $(OBJECTS)
cleanall: clean
