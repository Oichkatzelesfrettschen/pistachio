add_executable(memparse memparse.cc)

option(ENABLE_CAPNP "Link against libcapnp" OFF)
if(ENABLE_CAPNP)
    find_package(CapnProto REQUIRED)
    target_link_libraries(memparse PRIVATE CapnProto::capnp)
    target_compile_definitions(memparse PRIVATE WITH_CAPNP)
endif()

add_custom_target(memserver_tool ALL DEPENDS memparse)
