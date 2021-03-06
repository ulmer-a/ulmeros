#pragma once

#include <util/types.h>

#ifdef ARCH_X86_64

    struct arch_context_
    {
        size_t r15;
        size_t r14;
        size_t r13;
        size_t r12;
        size_t r11;
        size_t r10;
        size_t r9;
        size_t r8;
        size_t rdi;
        size_t rsi;
        size_t rdx;
        size_t rcx;
        size_t rbx;
        size_t rax;
        size_t rbp;
        size_t gs;
        size_t fs;
        size_t irq;
        size_t error;
        size_t rip;
        size_t cs;
        size_t rflags;
        size_t rsp;
        size_t ss;
    } __attribute__((packed));

#else

    #error fixme check if context struct is correct!

    struct arch_context_
    {
        size_t edi;
        size_t esi;
        size_t edx;
        size_t ecx;
        size_t ebx;
        size_t eax;
        size_t ebp;
        size_t gs;
        size_t fs;
        size_t irq;
        size_t error;
        size_t eip;
        size_t cs;
        size_t eflags;
        size_t esp;
        size_t ss;
    } __attribute__((packed));

#endif

typedef struct arch_context_ context_t;
