CFLAGS ?= -std=c2x
CXXFLAGS ?= -std=c++23
CPU_CFLAGS ?= -march=native
CFLAGS += $(CPU_CFLAGS)
CXXFLAGS += $(CPU_CFLAGS)
CFLAGS += -Werror
CXXFLAGS += -Werror

build/string.o: user/contrib/elf-loader/platform/amd64-pc99/string.cc
	echo Building string.o
	mkdir -p build
	g++ $(CXXFLAGS) -Iuser/include -Iuser/contrib/elf-loader/include -c $< -o $@

all: build/string.o

.PHONY: all clean check tests

build/tests:
	mkdir -p build/tests

build/tests/posix:
	mkdir -p build/tests/posix

build/tests/spinlock_fairness: tests/spinlock_fairness.c | build/tests
	$(CC) $(CFLAGS) -pthread $< -o $@

build/tests/posix/test_file: tests/posix/test_file.c | build/tests/posix
	$(CC) $(CFLAGS) $< -o $@

build/tests/posix/test_process: tests/posix/test_process.c user/lib/posix/posix.cc | build/tests/posix
	$(CXX) $(CXXFLAGS) -Iuser/lib/posix $^ -o $@

build/tests/posix/spawn_child: tests/posix/spawn_child.c user/lib/posix/posix.cc | build/tests/posix
	$(CXX) $(CXXFLAGS) -Iuser/lib/posix $^ -o $@

build/tests/posix/test_spawn_wait: tests/posix/test_spawn_wait.c user/lib/posix/posix.cc | build/tests/posix build/tests/posix/spawn_child
	$(CXX) $(CXXFLAGS) -Iuser/lib/posix $^ -o $@

tests: build/tests/spinlock_fairness build/tests/posix/test_file build/tests/posix/test_process build/tests/posix/test_spawn_wait

clean:
	rm -rf build
	find . -name '*.o' -delete

check: all tests
	python -m unittest discover -v tests
	./build/tests/spinlock_fairness
	./build/tests/posix/test_file
	./build/tests/posix/test_process
	./build/tests/posix/test_spawn_wait
