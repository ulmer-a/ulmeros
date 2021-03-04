# UlmerOS CMake TopLevel Configuration
# Recommended way of invocation is via 'menuconf'
# For more information, see README.md
cmake_minimum_required(VERSION 3.10)

# General project settings
project(UlmerOS VERSION 0.1 LANGUAGES C ASM)
message(STATUS "ulmeros: v${CMAKE_PROJECT_VERSION}")
enable_language(ASM)

# default configuration in cmake is release, activate
# debug configuration in menuconf.
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "configuration: ${CMAKE_BUILD_TYPE}")

if(NOT DEFINED ARCH OR NOT EXISTS "${PROJECT_SOURCE_DIR}/arch/${ARCH}")
    message(FATAL_ERROR "invalid architecture ${ARCH}")
endif()

if(NOT DEFINED PLATFORM OR NOT EXISTS "${PROJECT_SOURCE_DIR}/drivers/${PLATFORM}")
    message(FATAL_ERROR "invalid platform ${PLATFORM}")
endif()

message(STATUS "building for ${ARCH}-${PLATFORM}")

# Compiler choices
set(CMAKE_C_COMPILER "${ARCH}-elf-gcc")
set(CMAKE_ASM_COMPILER "${ARCH}-elf-gcc")

string(TOUPPER "ARCH_${ARCH}" ARCH_FLAG)
add_compile_definitions(
    $<$<CONFIG:DEBUG>:DEBUG>
    "${ARCH_FLAG}"
)

add_compile_options(
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:RELEASE>:-O2>
    -Wall -Werror -fvar-tracking
    -ffreestanding -fno-strict-aliasing
    -fno-builtin -fno-stack-protector
    -nostdlib -nostdinc -mno-sse2
)

add_link_options(
    -nostdlib
    -Wl,-nmagic
    -Wl,-gc-sections
    -Wl,--build-id=none
    -Wl,--no-relax
)

set(TARGET_INCLUDE
    "${PROJECT_SOURCE_DIR}/kernel/include"
    "${PROJECT_SOURCE_DIR}/arch/${ARCH}/include"
    "${PROJECT_SOURCE_DIR}/libutil/include"
)

include(arch/${ARCH}/${ARCH}.cmake)

add_subdirectory(drivers/${PLATFORM})
add_subdirectory(libutil)
add_subdirectory(kernel)
add_subdirectory(arch/${ARCH})
