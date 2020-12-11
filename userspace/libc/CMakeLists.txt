cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE LIBC_SOURCES "*.c")
file(GLOB_RECURSE LIBC_HEADERS "*.h")

add_library(libc STATIC ${LIBC_HEADERS} ${LIBC_SOURCES})

target_include_directories(libc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(libc PUBLIC
    ${USER_CFLAGS}
)