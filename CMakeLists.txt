cmake_minimum_required(VERSION 3.16)
project(pistachio C CXX)

# Add Eigen headers if present
include_directories(${CMAKE_SOURCE_DIR}/third_party/eigen)

# Allow passing cross build settings through CMake
set(TOOLPREFIX "" CACHE STRING "Prefix for cross compilation tools")
set(SUBARCH "" CACHE STRING "Kernel sub-architecture")
set(ARCH "" CACHE STRING "Kernel architecture")

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

add_compile_options(-Werror)

# CPU tuning flags. Default to -march=native but allow overrides via
# -DTUNE_CPU=<cpu> or explicit -DCMAKE_C_FLAGS_RELEASE.
set(TUNE_CPU "native" CACHE STRING "CPU microarchitecture for -march")
set(CPU_CFLAGS "-march=${TUNE_CPU}")
if("${CMAKE_C_FLAGS_RELEASE}" STREQUAL "")
    set(CMAKE_C_FLAGS_RELEASE "${CPU_CFLAGS}")
endif()
if("${CMAKE_CXX_FLAGS_RELEASE}" STREQUAL "")
    set(CMAKE_CXX_FLAGS_RELEASE "${CPU_CFLAGS}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

# Build the string.o example from the legacy Makefile
add_library(string_obj OBJECT
    user/contrib/elf-loader/platform/amd64-pc99/string.cc
)

target_include_directories(string_obj PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
)

add_subdirectory(kernel)
add_subdirectory(src-userland/lib)
add_subdirectory(tools/memserver)

add_custom_target(default_all ALL DEPENDS string_obj kernel_target userlib_target memserver_tool)

add_executable(spinlock_fairness tests/spinlock_fairness.c)
target_link_libraries(spinlock_fairness PRIVATE pthread)

add_executable(posix_test_file tests/posix/test_file.c)
add_executable(posix_test_process tests/posix/test_process.c)
add_executable(fifo_test user/apps/fifo_test/fifo_test.c)
target_link_libraries(fifo_test PRIVATE posix)

add_custom_target(tests DEPENDS spinlock_fairness posix_test_file posix_test_process fifo_test)

