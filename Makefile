CFLAGS ?= -std=c2x
CXXFLAGS ?= -std=c++23

build/string.o: user/contrib/elf-loader/platform/amd64-pc99/string.cc
	echo Building string.o
	mkdir -p build
	g++ $(CXXFLAGS) -Iuser/include -Iuser/contrib/elf-loader/include -c $< -o $@

all: build/string.o

.PHONY: all
