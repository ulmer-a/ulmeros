cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE CORE_SOURCES "*.c")
file(GLOB_RECURSE CORE_HEADERS "*.h")

add_library(newlib-core STATIC ${CORE_SOURCES} ${CORE_HEADERS})


target_include_directories(newlib-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(newlib-core PUBLIC
    ${NL_CFLAGS}
)
