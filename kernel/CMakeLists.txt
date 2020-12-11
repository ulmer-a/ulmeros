cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE KERNEL_SOURCES "*.c")
file(GLOB_RECURSE KERNEL_HEADERS "*.h")

add_executable(kernel ${KERNEL_SOURCES} ${KERNEL_HEADERS})

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
configure_file(${PROJECT_SOURCE_DIR}/kernel/include/version.h.in
    ${PROJECT_BINARY_DIR}/kernel/cmake_version.h
)

target_include_directories(kernel PUBLIC
    ${PROJECT_BINARY_DIR}/kernel
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libk/include"
    "${PROJECT_SOURCE_DIR}/${ARCH_PATH}"
)

target_compile_options(kernel PUBLIC
    ${ARCH_CFLAGS}
)

add_dependencies(kernel ${ARCH_TARGETS})
target_link_libraries(kernel PUBLIC ${ARCH_TARGETS})

set(KERNEL_ELF ${PROJECT_BINARY_DIR}/kernel/kernel)
set(KERNEL_O ${PROJECT_BINARY_DIR}/kernel/kernel.o)

add_custom_target(kernelbin
    objcopy -O binary ${KERNEL_ELF} kernel.bin
    COMMAND objcopy -I binary -O elf32-i386 -B i386 kernel.bin ${KERNEL_O}
    DEPENDS kernel
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_dependencies(kernelbin kernel)