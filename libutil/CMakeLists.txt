cmake_minimum_required(VERSION 3.14)

set(UTIL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB UTIL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/util/*.h")
file(GLOB UTIL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(util STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
target_include_directories(util PUBLIC
    ${UTIL_INCLUDE}
    ${PROJECT_SOURCE_DIR}/kernel/include
)

target_compile_options(util PUBLIC
    --target=${ARCH_PREFIX} -fPIC
)

# build the same target for use with
# boot32 stage of the amd64 boot loader
add_library(util32 STATIC ${UTIL_HEADERS} ${UTIL_SOURCES})
target_include_directories(util32 PUBLIC ${UTIL_INCLUDE})
target_compile_definitions(util32 PRIVATE BOOT32)

target_compile_options(util32 PUBLIC
    --target=${ARCH_PREFIX_32}
)

set(UTIL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
