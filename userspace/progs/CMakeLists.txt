cmake_minimum_required(VERSION 3.10)

file(GLOB USER_PROGS "*.c")

add_custom_target(progs)

set(USER_LDSCRIPT ${PROJECT_SOURCE_DIR}/userspace/libc/${ARCH}.ld)

message(STATUS "progs=${USER_PROGS}")
foreach(USER_PROG_PATH ${USER_PROGS})
    get_filename_component(USER_PROG_SRC ${USER_PROG_PATH} NAME_WLE)
    set(USER_PROG "${USER_PROG_SRC}.bin")

    add_dependencies(progs ${USER_PROG})

    add_executable(${USER_PROG} ${USER_PROG_SRC})
    target_link_libraries(${USER_PROG} PUBLIC libc)

    target_include_directories(${USER_PROG} PUBLIC
        "${PROJECT_SOURCE_DIR}/userspace/libc/include"
    )

    target_compile_options(${USER_PROG} PUBLIC
        ${USER_CFLAGS}
    )

    target_link_options(${USER_PROG} PUBLIC
        -T ${USER_LDSCRIPT} ${USER_LDFLAGS}
    )
endforeach()
