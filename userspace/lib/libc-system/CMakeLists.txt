cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE SYS_HEADERS "*.h")
file(GLOB_RECURSE SYS_SOURCES "*.c")
file(GLOB_RECURSE SYS_ASM_SOURCES "*.S")

add_library(libc-system ${SYS_SOURCES} ${SYS_HEADERS} ${SYS_ASM_SOURCES})

target_include_directories(libc-system PUBLIC
    ${USER_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../libc-system/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libc-system/include/pdclib
)

target_compile_definitions(libc-system PUBLIC
    -DULMEROS -D_PDCLIB_BUILD -D_PDCLIB_STATIC_DEFINE
)

target_compile_options(libc-system PUBLIC
    ${USER_CFLAGS}
)
