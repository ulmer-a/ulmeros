cmake_minimum_required(VERSION 3.10)

option(PC_DISKIMAGE_GEN "generate GRUB hdd diskimage" ON)

# build the kernel image
project(UlmerOS VERSION 0.1 LANGUAGES C ASM)

enable_language(ASM)

if(NOT DEFINED ARCH)
    set(ARCH "amd64")
endif(NOT DEFINED ARCH)

set(ARCH_CFLAGS_COMMON $<$<CONFIG:DEBUG>:-O0>  $<$<CONFIG:DEBUG>:-fvar-tracking>
    $<$<CONFIG:RELEASE>:-O2> -ffreestanding -nostdinc -fno-strict-aliasing
    -fno-builtin -fno-stack-protector)

set(ARCH_LDFLAGS_COMMON -nostdlib LINKER:--warn-common LINKER:-no-relax)

if ("${ARCH}" STREQUAL "amd64")
    set(ARCH_TARGET "arch/x86")
    set(ARCH_TARGETS x86_common amd64 platform-pc)
    set(ARCH_PATH "arch/x86/${ARCH}")
    set(ARCH_CFLAGS ${ARCH_CFLAGS_COMMON} -mno-red-zone)
    set(PLATFORM "pc")
endif ("${ARCH}" STREQUAL "amd64")

if ("${ARCH}" STREQUAL "i386")
    set(ARCH_TARGET "arch/x86")
    set(ARCH_TARGETS x86_common i386 platform-pc)
    set(ARCH_PATH "arch/x86/${ARCH}")
    set(ARCH_CFLAGS -m32 ${ARCH_CFLAGS_COMMON})
    set(PLATFORM "pc")
endif ("${ARCH}" STREQUAL "i386")

set(CMAKE_BUILD_TYPE "DEBUG")

add_subdirectory(kernel)
add_subdirectory(${ARCH_TARGET})
add_subdirectory("drivers/${PLATFORM}")
add_subdirectory(userspace)

set(KERNEL_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/kernel/kernel.ld)
target_link_options(kernel PUBLIC -T ${KERNEL_LDSCRIPT} ${ARCH_LDFLAGS_COMMON})

add_custom_target(qemu
    tools/qemu.sh ${PROJECT_BINARY_DIR}/disk.img
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)