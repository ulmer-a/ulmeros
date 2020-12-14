cmake_minimum_required(VERSION 3.10)

set(ARCH amd64)

set(USER_CFLAGS $<$<CONFIG:DEBUG>:-O0>  $<$<CONFIG:DEBUG>:-fvar-tracking>
    $<$<CONFIG:RELEASE>:-O2> -ffreestanding -nostdinc -fno-strict-aliasing
    -fno-builtin -fno-stack-protector)

set(USER_LDFLAGS -nostdlib LINKER:--warn-common LINKER:-no-relax)

set(USER_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/libc-system/include"
)

add_subdirectory(lib)
add_subdirectory(progs)

add_custom_target(userspace)
add_dependencies(userspace libc-core libc-system progs)
