CXX ?= c++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test demo clean
all: build/witness
build/witness: witness.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) witness.cpp -o build/witness
test: build/witness
	python3 -m unittest -v
demo: build/witness
	./build/witness --demo
clean:
	rm -rf build
