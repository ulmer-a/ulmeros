cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE KERNEL_SOURCES "*.c")
file(GLOB_RECURSE KERNEL_HEADERS "*.h")

add_library(kernel ${KERNEL_SOURCES} ${KERNEL_HEADERS})
target_compile_options(kernel PRIVATE ${TARGET_CFLAGS})
target_include_directories(kernel PRIVATE ${TARGET_INCLUDE})
target_link_libraries(kernel util)
