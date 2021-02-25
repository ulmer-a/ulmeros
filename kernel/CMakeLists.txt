cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE KERNEL_SOURCES "*.c")
file(GLOB_RECURSE KERNEL_HEADERS "*.h")

add_library(kernel ${KERNEL_SOURCES} ${KERNEL_HEADERS})

target_compile_options(kernel PRIVATE
    --target=${ARCH_PREFIX} -fPIC -mno-red-zone
)

target_include_directories(kernel PRIVATE
    "${UTIL_INCLUDE}"
    "${PROJECT_SOURCE_DIR}/arch/${ARCH}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(kernel util)

set(KERNEL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
