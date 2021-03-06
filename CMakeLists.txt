cmake_minimum_required(VERSION 3.14)

project(UlmerOSDist VERSION 1.0)

set(SYSROOT "${PROJECT_BINARY_DIR}/rootfs")

# Build and install the UlmerOS kernel
add_subdirectory(kernel)
add_custom_target(kernel_install ALL
	mkdir -p ${SYSROOT}/boot
	COMMAND cp ${PROJECT_BINARY_DIR}/vmulmer ${SYSROOT}/boot/vmulmer
	DEPENDS kernel_img
)
add_custom_target(userspace DEPENDS kernel_img)

# Build the Newlib C standard library
option(U_NEWLIB "Build Newlib C standard library" OFF)
add_subdirectory(newlib)

# Build a PC disk image
option(U_PCIMG "build a PC disk image" OFF)
option(U_GRUB "install GRUB2" OFF)
add_subdirectory(diskimage)

# Build /bin/init program
option(U_INIT "build /bin/init program" OFF)
add_subdirectory(init)

