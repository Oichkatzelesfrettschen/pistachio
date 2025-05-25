cmake_minimum_required(VERSION 3.20)

# Require clang/LLVM toolchain
if(NOT CMAKE_C_COMPILER MATCHES "clang$" AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Clang is required to build Pistachio")
endif()
if(NOT CMAKE_CXX_COMPILER MATCHES "clang\+\+$" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Clang++ is required to build Pistachio")
endif()

project(pistachio C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

foreach(lang C CXX)
    set(CMAKE_${lang}_STANDARD 23)
    set(CMAKE_${lang}_STANDARD_REQUIRED ON)
endforeach()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")

# Add Eigen headers if present
include_directories(${CMAKE_SOURCE_DIR}/third_party/eigen)

# Allow passing cross build settings through CMake
set(TOOLPREFIX "" CACHE STRING "Prefix for cross compilation tools")
set(SUBARCH "" CACHE STRING "Kernel sub-architecture")
set(ARCH "" CACHE STRING "Kernel architecture")


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
add_subdirectory(user/apps/ipc_demo)

add_custom_target(default_all ALL DEPENDS string_obj kernel_target userlib_target memserver_tool ipc_demo_apps)

add_executable(spinlock_fairness tests/spinlock_fairness.c)
target_link_libraries(spinlock_fairness PRIVATE pthread)

add_executable(posix_test_file tests/posix/test_file.c)
add_executable(posix_test_process tests/posix/test_process.c)
add_executable(posix_dirlist user/apps/dirlist.c)

add_custom_target(tests DEPENDS spinlock_fairness posix_test_file posix_test_process posix_dirlist)

