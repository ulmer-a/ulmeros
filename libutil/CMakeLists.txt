cmake_minimum_required(VERSION 3.14)

file(GLOB UTIL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.h")
file(GLOB UTIL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(util STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
target_include_directories(util PUBLIC
    ${TARGET_INCLUDE}
    ${PROJECT_SOURCE_DIR}/kernel/include
)
target_compile_options(util PUBLIC ${TARGET_CFLAGS})

# build the same target for use with
# boot32 stage of the amd64 boot loader
add_library(util32 STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
target_include_directories(util32 PUBLIC ${TARGET_INCLUDE})
target_compile_definitions(util32 PRIVATE BOOT32)
target_compile_options(util32 PUBLIC -m32 -mno-sse2)
