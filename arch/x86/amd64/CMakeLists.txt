cmake_minimum_required(VERSION 3.10)

file(GLOB AMD64_SOURCES "*.c")
file(GLOB AMD64_HEADERS "*.h")
file(GLOB AMD64_ASM_SOURCES "*.S")

add_library(amd64 STATIC ${AMD64_SOURCES} ${AMD64_ASM_SOURCES} ${AMD64_HEADERS})

target_include_directories(amd64 PUBLIC
    ${PROJECT_BINARY_DIR}/kernel
    "${PROJECT_SOURCE_DIR}/arch/x86/include"
    "${PROJECT_SOURCE_DIR}/kernel/include"
    "${PROJECT_SOURCE_DIR}/kernel/libk/include"
)

target_compile_options(amd64 PUBLIC
    ${ARCH_CFLAGS}
)

add_subdirectory(boot32)