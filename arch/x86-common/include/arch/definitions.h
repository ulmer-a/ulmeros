#pragma once

/* definitions for the x86 hardware architecture. */

#define PAGE_SIZE           4096
#define PAGE_SHIFT          12

#ifdef ARCH_X86_64

    #define KHEAP_START     0xffffe00000000000ul
    #define KHEAP_END       0xfffffffffffffffful
    #define IDENT_OFFSET    0xffff800000000000ul
    #define USER_BREAK      0x0000800000000000ul

    /* ELF loader for architecture for comparison
    * with values in the ELF header */
    #define ELF_WORDSIZE      2       // 2 -> 64 bit
    #define ELF_ENDIANNESS    1       // 1 -> little endian
    #define ELF_ISA           0x3e    // 0x3e -> x86_64

#else

    #define KHEAP_START     0xc0000000ul
    #define KHEAP_END       0xfffffffful
    #define USER_BREAK      0x80000000ul

    /* ELF loader for architecture for comparison
    * with values in the ELF header */
    #define ELF_WORDSIZE      1       // 1 -> 32 bit
    #define ELF_ENDIANNESS    1       // 1 -> little endian
    #define ELF_ISA           3       // 3 -> i386

#endif