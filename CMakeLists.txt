cmake_minimum_required(VERSION 3.10)

option(PC_DISKIMAGE_GEN "generate GRUB hdd diskimage" ON)

# build the kernel image
project(UlmerOS VERSION 0.1 LANGUAGES C ASM)
message(STATUS "ulmeros: building v${CMAKE_PROJECT_VERSION}")

enable_language(ASM)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_ASM_COMPILER "clang")



# Default build type
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "ulmeros: building ${CMAKE_BUILD_TYPE} configuration")

if(NOT DEFINED ARCH)
    set(ARCH "x86")
    set(ARCH_PREFIX "x86_64-elf")
    set(ARCH_PREFIX_32 "i386-elf")
endif(NOT DEFINED ARCH)


add_compile_options(
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:RELEASE>:-O2>
    -ffreestanding -nostdinc -fno-strict-aliasing
    -fno-builtin -fno-stack-protector
)

add_link_options(
    -nostdlib -Wl,-nmagic -Wl,-gc-sections
)

add_subdirectory(libutil)
add_subdirectory(arch/${ARCH})
