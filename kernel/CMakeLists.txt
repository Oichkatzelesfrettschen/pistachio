# Wrap the existing kernel Makefile

set(TOOLPREFIX "" CACHE STRING "Prefix for cross compilation tools")
set(SUBARCH "" CACHE STRING "Kernel sub-architecture")
set(ARCH "" CACHE STRING "Kernel architecture")

add_custom_target(kernel_target ALL
    COMMAND ${CMAKE_MAKE_PROGRAM} TOOLPREFIX=${TOOLPREFIX} SUBARCH=${SUBARCH} ARCH=${ARCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building kernel via Makefile"
)

add_custom_target(kernel_clean
    COMMAND ${CMAKE_MAKE_PROGRAM} TOOLPREFIX=${TOOLPREFIX} SUBARCH=${SUBARCH} ARCH=${ARCH} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning kernel build"
)
