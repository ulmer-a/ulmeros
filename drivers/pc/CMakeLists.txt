cmake_minimum_required(VERSION 3.14)

file(GLOB PC_SOURCES "*.c")
file(GLOB PC_HEADERS "*.h")

add_library(platform STATIC
    ${PC_SOURCES}
    ${PC_HEADERS}
)

target_include_directories(platform PUBLIC
    ${UTIL_INCLUDE}
    ${PROJECT_SOURCE_DIR}/kernel/include
    ${PROJECT_SOURCE_DIR}/arch/${ARCH}/include
)

target_compile_options(platform PUBLIC
    --target=${ARCH_PREFIX} -fPIC -mno-red-zone
)
