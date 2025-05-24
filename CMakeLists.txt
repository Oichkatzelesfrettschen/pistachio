cmake_minimum_required(VERSION 3.16)
project(pistachio C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

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

add_custom_target(default_all ALL DEPENDS string_obj kernel_target userlib_target)

