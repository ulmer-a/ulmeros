cmake_minimum_required(VERSION 3.10)

file(GLOB BOOT32_SOURCES "*.c")
file(GLOB BOOT32_ASM_SOURCES "*.S")
file(GLOB BOOT32_HEADERS "*.h")
set(KERNEL_O ${PROJECT_BINARY_DIR}/kernel/kernel.o)

add_executable(boot32 ${BOOT32_SOURCES} ${BOOT32_ASM_SOURCES} ${BOOT32_HEADERS})

# boot32 stage configuration
set(BOOT32_LOAD_ADDR 0xdeadbeef)
set(BOOT32_WELCOME "UlmerOS boot32 stage")
set(BOOT32_IDMAP_SIZE 134217728)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/boot32config.h.in
    ${PROJECT_BINARY_DIR}/boot32config.h
)

target_include_directories(boot32 PUBLIC
    ${PROJECT_BINARY_DIR}
    "${PROJECT_SOURCE_DIR}/arch/x86/include"
    "${PROJECT_SOURCE_DIR}/arch/x86/amd64/boot32"
)

target_compile_options(boot32 PUBLIC
    -m32 ${ARCH_CFLAGS_COMMON}
)

set(BOOT32_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boot32.ld)

target_link_options(boot32 PUBLIC -T ${BOOT32_LDSCRIPT} ${ARCH_LDFLAGS_COMMON} ${KERNEL_O})

add_dependencies(boot32 kernelbin)