cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE KERNEL_SOURCES "*.c")
file(GLOB_RECURSE KERNEL_HEADERS "*.h")

add_library(kernel STATIC ${KERNEL_SOURCES} ${KERNEL_HEADERS})

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
configure_file(${PROJECT_SOURCE_DIR}/kernel/include/version.h.in
    ${PROJECT_BINARY_DIR}/kernel/cmake_version.h
)

target_include_directories(kernel PUBLIC
    ${PROJECT_BINARY_DIR}/kernel
    "${PROJECT_SOURCE_DIR}/kernel/include"
    "${PROJECT_SOURCE_DIR}/kernel/libk/include"
    "${PROJECT_SOURCE_DIR}/arch/${ARCH}"
)

target_compile_options(kernel PUBLIC
    $<$<CONFIG:DEBUG>:-O0>  $<$<CONFIG:DEBUG>:-fvar-tracking>
    $<$<CONFIG:RELEASE>:-O2>
    -ffreestanding -nostdinc -fno-strict-aliasing
    -fno-builtin -fno-stack-protector -mno-red-zone
)