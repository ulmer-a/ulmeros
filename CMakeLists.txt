cmake_minimum_required(VERSION 3.14)

project(UlmerOSDist VERSION 1.0)

# build the kernel 
add_subdirectory(kernel)

option(U_INIT "build /bin/init program" OFF)
if(U_INIT)
    message(STATUS "building init")
endif()

option(U_GRUB "install GRUB2 bootloader" OFF)
if(U_GRUB)
    message(STATUS "installing GRUB")
endif()
