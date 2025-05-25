add_library(capnp STATIC
    capnp.c
)

target_include_directories(capnp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
