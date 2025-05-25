# Build userland static libraries

add_library(useripc STATIC
    ${CMAKE_SOURCE_DIR}/user/lib/ipc/user_ipc.cc
)

add_library(sched STATIC
    ${CMAKE_SOURCE_DIR}/user/lib/sched/sched_client.cc
)

# Include directories match the Makefile
# using absolute paths from the repository root

target_include_directories(useripc PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
)

target_include_directories(sched PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
)

set(USERLIB_TARGET_DEPS useripc sched)

if(WITH_CAPNP)
    add_subdirectory(libcapnp)
    list(APPEND USERLIB_TARGET_DEPS capnp)
endif()

add_custom_target(userlib_target ALL
    DEPENDS ${USERLIB_TARGET_DEPS}
)

add_custom_target(userlib_clean
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
