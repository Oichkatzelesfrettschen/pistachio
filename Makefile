# Use ccache if available to speed up rebuilds.
CCWRAP := $(shell command -v ccache)

ifdef CCWRAP
    # Prepend ccache to the compiler command when found.
    CC ?= $(CCWRAP) clang
    CXX ?= $(CCWRAP) clang++
else
    # Fallback to plain clang when ccache is not installed.
    CC ?= clang
    CXX ?= clang++
endif

# Default compilation flags for C and C++ files.
CFLAGS ?= -std=c2x
CXXFLAGS ?= -std=c++23
# Optimize for the native CPU.
CPU_CFLAGS ?= -march=native

# Apply common flags to both C and C++ builds.
CFLAGS += $(CPU_CFLAGS)
CXXFLAGS += $(CPU_CFLAGS)
CFLAGS += -Werror
CXXFLAGS += -Werror

# Build the C++ string implementation used by the bootstrap loader.
build/string.o: engine/include/user/contrib/elf-loader/platform/amd64-pc99/string.cc
	echo Building string.o
	mkdir -p build
	$(CXX) $(CXXFLAGS) -Iengine/include/user/include -Iengine/include/user/contrib/elf-loader/include -c $< -o $@

all: build/string.o

.PHONY: all clean check tests

# Directory for compiled test binaries.
build/tests:
	mkdir -p build/tests

# Directory for POSIX compatibility tests.
build/tests/posix:
	mkdir -p build/tests/posix

# Build the spinlock fairness benchmark used for performance regression tests.
build/tests/spinlock_fairness: engine/include/tests/spinlock_fairness.c | build/tests
	# Compile the spinlock benchmark using pthreads support.
	$(CC) $(CFLAGS) -pthread $< -o $@
	# Verify correct file handling operations on a POSIX-like environment.
build/tests/posix/test_file: engine/include/tests/posix/test_file.c | build/tests/posix
	# Build the POSIX file operation test program.
	$(CC) $(CFLAGS) $< -o $@
	# Check fork and exec behaviour for basic process management.
build/tests/posix/test_process: engine/include/tests/posix/test_process.c | build/tests/posix
	# Compile the basic fork/exec behaviour checker.
	$(CC) $(CFLAGS) $< -o $@
	# Build a directory listing utility for tests.
build/tests/posix/dirlist: engine/include/user/apps/dirlist.c | build/tests/posix
	# Compile a simple directory listing utility used by tests.
	$(CC) $(CFLAGS) $< -o $@
# Aggregate all test binaries into the "tests" target
tests: build/tests/spinlock_fairness build/tests/posix/test_file build/tests/posix/test_process build/tests/posix/dirlist
clean:
	rm -rf build
	find . -name '*.o' -delete

check: all tests
	python -m unittest discover -v tests
	./build/tests/spinlock_fairness
	./build/tests/posix/test_file
	./build/tests/posix/test_process
	./build/tests/posix/dirlist
