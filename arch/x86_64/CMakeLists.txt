cmake_minimum_required(VERSION 3.14)

# sources fot the boot32 boot stage
file(GLOB X86_BOOT32_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/boot32/*.h")
file(GLOB X86_BOOT32_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/boot32/*.c")
file(GLOB X86_BOOT32_ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/boot32/*.S")

# sources for the 64bit kernel
file(GLOB X86_COMMON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/x86/*.h")
file(GLOB X86_COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/x86/*.c")
file(GLOB X86_64_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/*.h")
file(GLOB X86_64_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/*.c")
file(GLOB X86_64_ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/*.S")

option(A_NX "enable support for NX pages" OFF)
if(A_NX)
    message(STATUS "x86_64: enable NX support")
    add_compile_definitions(ARCH_NX)
endif()

# ----------- x86 ARCH CODE  ---------------
add_executable(arch
    ${X86_COMMON_SOURCES}
    ${X86_COMMON_HEADERS}
    ${X86_64_HEADERS}
    ${X86_64_SOURCES}
    ${X86_64_ASM_SOURCES}
)

target_link_libraries(arch util kernel platform)

target_link_options(arch PRIVATE
    -Wl,-T "${CMAKE_CURRENT_SOURCE_DIR}/../x86-common/x86_64.ld"
)

target_compile_options(arch PRIVATE ${TARGET_CFLAGS})

target_include_directories(arch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(arch
    PROPERTIES
    OUTPUT_NAME "kernel64.elf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(arch PRIVATE
    ${TARGET_INCLUDE}
)

# ---------- x86 BOOT32 STAGE --------------

add_custom_target(kernelbin
    objcopy -O binary ${CMAKE_BINARY_DIR}/kernel64.elf kernel.bin
    COMMAND objcopy -I binary -O elf32-i386 -B i386 kernel.bin kernel64.o
    DEPENDS arch
    BYPRODUCTS ${CMAKE_BINARY_DIR}/kernel64.o
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_executable(boot32
    ${X86_BOOT32_SOURCES}
    ${X86_BOOT32_HEADERS}
    ${X86_BOOT32_ASM_SOURCES}
    ${CMAKE_BINARY_DIR}/kernel64.o
)
add_dependencies(boot32 kernelbin)

target_link_libraries(boot32 util32)
target_compile_options(boot32 PRIVATE
    "--target=i386-elf"
)
target_link_options(boot32 PRIVATE
    -Wl,-T "${CMAKE_CURRENT_SOURCE_DIR}/../x86-common/i386.ld"
)

set_target_properties(boot32
    PROPERTIES
    OUTPUT_NAME "vmulmer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
