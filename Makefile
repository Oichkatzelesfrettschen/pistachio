CFLAGS ?= -std=c2x
CXXFLAGS ?= -std=c++23
CPU_CFLAGS ?= -march=native
CFLAGS += $(CPU_CFLAGS)
CXXFLAGS += $(CPU_CFLAGS)
WITH_CAPNP ?= 0

build/string.o: user/contrib/elf-loader/platform/amd64-pc99/string.cc
	echo Building string.o
	mkdir -p build
	g++ $(CXXFLAGS) -Iuser/include -Iuser/contrib/elf-loader/include -c $< -o $@



CAPNP_OBJ = build/libcapnp.o
CAPNP_LIB = build/libcapnp.a

$(CAPNP_OBJ): src-userland/lib/libcapnp/capnp.c
	mkdir -p build
	$(CC) $(CFLAGS) -Isrc-userland/lib/libcapnp -c $< -o $@

$(CAPNP_LIB): $(CAPNP_OBJ)
	ar rcs $@ $^

ifeq ($(WITH_CAPNP),1)
  BUILD_CAPNP = $(CAPNP_LIB)
endif

all: build/string.o $(BUILD_CAPNP)

.PHONY: all clean check tests

build/tests:
	mkdir -p build/tests

build/tests/spinlock_fairness: tests/spinlock_fairness.c | build/tests
	$(CC) $(CFLAGS) -pthread $< $(BUILD_CAPNP) -o $@

tests: build/tests/spinlock_fairness $(BUILD_CAPNP)

clean:
	rm -rf build
	find . -name '*.o' -delete

check: all tests
	python -m unittest discover -v tests
	./build/tests/spinlock_fairness
