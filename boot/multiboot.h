#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "boot32.h"

struct mb_struct
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    void*    mods;
    uint32_t syms[4];
    uint32_t mmap_length;
    void*    mmap;
    uint32_t drives_length;
    void*    drives;
    uint32_t config_table;
    char*    bootloader_name;
    uint32_t apmTable;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed));

struct modse_struct
{
    uint32_t mod_start;
    uint32_t mod_end;
    unsigned char string[4];
    unsigned char pad0[4];
} __attribute__((packed));

struct mmape_struct
{
    uint32_t entry_size;
    uint64_t base_addr;
    uint64_t size;
    uint32_t type;
} __attribute__((packed));

#endif
