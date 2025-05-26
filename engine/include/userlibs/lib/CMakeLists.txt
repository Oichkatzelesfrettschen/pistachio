# Build userland static libraries

add_library(useripc STATIC
    ${CMAKE_SOURCE_DIR}/user/lib/ipc/user_ipc.cc
    ${CMAKE_SOURCE_DIR}/user/lib/exo/exo_ipc.cc
)

add_library(sched STATIC
    ${CMAKE_SOURCE_DIR}/user/lib/sched/sched_client.cc
)

add_library(mlp STATIC
    ${CMAKE_SOURCE_DIR}/user/lib/mlp/mlp.cc
)

# Include directories match the Makefile
# using absolute paths from the repository root

target_include_directories(useripc PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
)

target_include_directories(sched PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
)

target_include_directories(mlp PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/user/contrib/elf-loader/include
    ${CMAKE_SOURCE_DIR}/third_party/eigen
)

add_custom_target(userlib_target ALL
    DEPENDS useripc sched mlp
)

add_custom_target(userlib_clean
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
