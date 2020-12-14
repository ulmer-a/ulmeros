cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE LIBC_CORE_SOURCES "*.c")
file(GLOB_RECURSE LIBC_CORE_HEADERS "*.h")

add_library(libc-core ${LIBC_CORE_SOURCES} ${LIBC_CORE_HEADERS})

add_dependencies(libc-core libc-system)
target_link_libraries(libc-core libc-system)

target_include_directories(libc-core PUBLIC
    ${USER_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../libc-system/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../libc-system/include/pdclib
)

target_compile_definitions(libc-core PUBLIC
    -DULMEROS -D_PDCLIB_BUILD -D_PDCLIB_STATIC_DEFINE
    -DHAVE_MMAP=0
    -DLACKS_SYS_MMAN_H -DLACKS_SCHED_H -DLACKS_SYS_PARAM_H
)

target_compile_options(libc-core PUBLIC
    ${USER_CFLAGS}
)
