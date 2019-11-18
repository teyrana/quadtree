CXX ?= g++
AR ?= ar
PREFIX = /usr/local

BUILD_DIR = build/
TEST_EXE = bin/test
BENCHMARK_EXE = bin/benchmark
QUAD_TREE_LIB = lib/libquadtree.a

HEADER = include/bounds.hpp include/point.hpp include/quad_node.hpp include/quad_tree.hpp
SOURCE = src/bounds.cpp src/point.cpp src/quad_node.cpp src/quad_tree.cpp

CPPFLAGS = -O2 -std=c++17 -Wall -g -pedantic -Iinclude/
LINKFLAGS =

OBJ = $(SOURCE:.cpp=.o)
OBJ = $(SOURCE:src/%.cpp=build/%.o)

default: test

lib: $(QUAD_TREE_LIB)
	$(AR) -cr $(LIBFLAGS) $(QUAD_TREE_LIB) $(OBJ)

$(QUAD_TREE_LIB): $(OBJ)
	$(AR) -cr $(LIBFLAGS) $(QUAD_TREE_LIB) $(OBJ)

$(TEST_EXE): $(QUAD_TREE_LIB) build/test.o 
	$(CXX) $^ -lm -o $@

$(BENCH_EXE): $(QUAD_TREE_LIB) build/benchmark.o 
	$(CXX) $^ -lm -o $@

clean:
	rm -rf build/*

$(OBJ): build/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) $< -c -o $@

%.o: src/%.c
	$(CXX) $< $(CPPFLAGS) -c -o $@

test: $(TEST_EXE)
	./$<

benchmark: $(BENCHMARK_EXE)
	./$<

.PHONY: benchmark bench clean default lib test
