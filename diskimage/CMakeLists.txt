cmake_minimum_required(VERSION 3.14)

if(U_PCIMG)
    message(STATUS "building PC disk image")

    if(U_GRUB)
        set(GRUB_INST "--grub=on")
    endif()

    add_custom_target(pc_diskimg ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/mkimg ${SYSROOT} ${CMAKE_CURRENT_SOURCE_DIR} ${GRUB_INST}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS userspace)
    
    
    add_custom_target(qemu
        qemu-system-${ARCH} -debugcon stdio -drive format=raw,file=disk.img -m 512 -s
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )

endif()
