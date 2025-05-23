# Build userland static libraries

add_library(useripc STATIC
    ipc/user_ipc.cc
)

add_library(sched STATIC
    sched/sched_client.cc
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

add_custom_target(userlib_target ALL
    DEPENDS useripc sched
)

add_custom_target(userlib_clean
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
