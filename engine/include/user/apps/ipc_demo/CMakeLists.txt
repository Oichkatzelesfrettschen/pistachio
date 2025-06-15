add_executable(self_ipc self_ipc/main.cc)
add_executable(typed_channel_demo
    typed_channel/main.cc
    ../../crypto/pqcrypto.cpp
)

target_include_directories(self_ipc PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(typed_channel_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/user/include
    ${CMAKE_SOURCE_DIR}/include
)

add_custom_target(ipc_demo_apps ALL DEPENDS self_ipc typed_channel_demo)
