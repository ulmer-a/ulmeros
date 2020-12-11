cmake_minimum_required(VERSION 3.10)

# build the kernel image
project(UlmerOS VERSION 0.1)

if(NOT DEFINED ARCH)
    set(ARCH "x86/amd64")
endif(NOT DEFINED ARCH)

set(CMAKE_BUILD_TYPE "DEBUG")

add_subdirectory(kernel)
