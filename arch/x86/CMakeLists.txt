cmake_minimum_required(VERSION 3.10)

file(GLOB X86_COMMON_SOURCES "*.c")
file(GLOB X86_COMMON_HEADERS "*.h")

add_library(x86_common STATIC ${X86_COMMON_SOURCES} ${X86_COMMON_HEADERS})

target_include_directories(x86_common PUBLIC
    ${PROJECT_BINARY_DIR}/kernel
    "${PROJECT_SOURCE_DIR}/arch/x86/include"
    "${PROJECT_SOURCE_DIR}/kernel/include"
    "${PROJECT_SOURCE_DIR}/kernel/libk/include"
)

target_compile_options(x86_common PUBLIC
    ${ARCH_CFLAGS}
)

# process architecture flavour dependent code
# (either i386 or amd64)
add_subdirectory(${ARCH})