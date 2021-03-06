cmake_minimum_required(VERSION 3.14)

file(GLOB UTIL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.h")
file(GLOB UTIL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(util STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
target_include_directories(util PUBLIC  ${TARGET_INCLUDE})
target_compile_options(util PUBLIC ${TARGET_CFLAGS})

if("${ARCH}" STREQUAL "x86_64")
    # On x86_64 we need to build a 32bit boot stage, which also
    # needs to have access to libutil. Therefore, libutil has to
    # be compiled twice, a 64bit version as well as a 32bit one.
    add_library(util32 STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
    target_include_directories(util32 PUBLIC ${TARGET_INCLUDE})
    target_compile_definitions(util32 PRIVATE BOOT32)
    target_compile_options(util32 PUBLIC -m32 -mno-mmx -mno-sse -mno-sse2)
endif()
