#pragma once

#define ARCH_KHEAP_START  ((void*)(0xffffe00000000000ul))
#define USER_BREAK        0x800000000000ul

#define ELF_ARCH_ENDIAN   1     // little endian
#define ELF_ARCH_ID       0x3e  // x86_64
#define ELF_ARCH_BIT_TYPE 2     // 64 bit
