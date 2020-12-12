cmake_minimum_required(VERSION 3.14)

file(GLOB USER_PROGS "*.c")

add_custom_target(progs)

add_dependencies(progs libc-core libc-system)

set(USER_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libc-system/arch/${ARCH}.ld)
foreach(USER_PROG_PATH ${USER_PROGS})
    get_filename_component(USER_PROG_SRC ${USER_PROG_PATH} NAME_WLE)
    set(USER_PROG "${USER_PROG_SRC}.bin")


    add_executable(${USER_PROG} ${USER_PROG_SRC})
    target_link_libraries(${USER_PROG} PUBLIC
        libc-system libc-core
        libc-system libc-core
        libc-system libc-core)

    target_include_directories(${USER_PROG} PUBLIC
        "${USER_INCLUDES}"

    )

    target_compile_options(${USER_PROG} PUBLIC
        ${USER_CFLAGS}
    )

    target_link_options(${USER_PROG} PUBLIC
        -T ${USER_LDSCRIPT} ${USER_LDFLAGS}
    )
endforeach()
