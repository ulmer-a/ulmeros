cmake_minimum_required(VERSION 3.14)

option(D_ATA "compile PCI ATA hard disk driver driver" OFF)
option(D_PS2KBD "compile PS/2 keyboard driver" OFF)
option(D_VGACON "compile VGA console" OFF)
option(D_SERIAL "compile Serial port driver" OFF)

set(PC_SOURCES "init.c" "pci.c")
add_library(platform STATIC ${PC_SOURCES})
target_include_directories(platform PUBLIC ${TARGET_INCLUDE})
target_compile_options(platform PUBLIC ${TARGET_CFLAGS})

message(STATUS "driver: PCI configuration space access")

if(D_ATA)
    message(STATUS "driver: PCI ATA Hard Disk (DMA)")
    target_sources(platform PRIVATE "ata.c")
    add_definitions(-DD_ATA)
endif()

if(D_PS2KBD)
    message(STATUS "driver: PS/2 keyboard")
    target_sources(platform PRIVATE "ps2kbd.c")
    add_definitions(-DD_PS2KBD)
endif()

if (D_VGACON)
    message(STATUS "driver: VGA console")
    target_sources(platform PRIVATE "vgacon.c")
    add_definitions(-DD_VGACON)
endif()

if (D_SERIAL)
    message(STATUS "driver: Serial port")
    target_sources(platform PRIVATE "serial.c")
    add_definitions(-DD_SERIAL)
endif()