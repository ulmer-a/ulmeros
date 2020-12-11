cmake_minimum_required(VERSION 3.10)

file(GLOB PC_SOURCES "*.c")
file(GLOB PC_HEADERS "*.h")

add_library(platform-pc ${PC_SOURCES} ${PC_HEADERS})

target_include_directories(platform-pc PUBLIC
    ${PROJECT_BINARY_DIR}/kernel
    "${PROJECT_SOURCE_DIR}/kernel/include"
    "${PROJECT_SOURCE_DIR}/kernel/libk/include"
    "${PROJECT_SOURCE_DIR}/${ARCH_PATH}"
    "${PROJECT_SOURCE_DIR}/arch/x86/include"
)

target_compile_options(platform-pc PUBLIC
    ${ARCH_CFLAGS}
)

add_custom_target(diskimage ALL
    ${PROJECT_SOURCE_DIR}/tools/mkimg.sh ${PROJECT_SOURCE_DIR}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
add_dependencies(diskimage vmulmer userspace)