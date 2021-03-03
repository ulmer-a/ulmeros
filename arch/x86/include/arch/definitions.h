#pragma once

#define KHEAP_START     0xffffe00000000000ul
#define USER_BREAK      0x0000800000000000ul

#define PAGE_SIZE   4096
#define PAGE_SHIFT  12

/* ELF loader for architecture for comparison
 * with values in the ELF header */
#define ELF_WORDSIZE      2       // 2 -> 64 bit
#define ELF_ENDIANNESS    1       // 1 -> little endian
#define ELF_ISA           0x3e    // 0x3e -> x86_64
