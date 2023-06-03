CC=gcc
CXX=g++
CFLAGS=-Wall $(OPT) -fPIC -std=c++11 -ggdb -O3
CXXFLAGS= $(CFLAGS)
TARGET_EXE=./bin
TEST_SRC=./src/hdf5.cpp
TEST_DIR=./tests

.PHONY: all, clean, test

all:
	@echo "done nothing"

clean: build
	@rm -f ./build/*
	@echo "Cleaning ./build/ ..."
	@echo "Done!"
	@rm -f ./bin/*
	@echo "Cleaning ./bin/ ..."
	@echo "Done!"
	@rm -f ./tests/*
	@echo "Cleaning ./tests/ ..."
	@echo "Done!"

build:
	mkdir build

ICconvert: ./src/ICconvert.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET_EXE)/ICconvert -lhdf5 $^

test: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_DIR)/test $< -lhdf5
