/*
 * ULMER Operating System
 * Architecture initialization code for amd64
 * Copyright (C) 2019.
 * Written by Alexander Ulmer <ulmer@student.tugraz.at>
 */

.section .text.init

    /* we're still executing at address 0x1000000 (16MB),
     * which is in userspace. that means we need to jump above the
     * user break, where the entire physical memory is mapped. */
    movabs $_start64, %rax
    jmp *%rax

.global _start64
_start64:
    /* make sure interrupts are disabled */
    cli

    /* setup the initilization stack */
    movabs $0xffff800001000000, %rsp

    /* enable SSE registers */
    mov %cr0, %rax
    btr $2, %rcx
    btsq $1, %rax
    mov %rax, %cr0
    mov %cr4, %rax
    btsq $9, %rax
    btsq $10, %rax
    mov %rax, %cr4

    /* a pointer to the boot info structure is
     * located in the RDI register. save it. */
    push %rdi

    /* call the architecture initialization function. this
     * will setup anything architecture-specific the kernel
     * reqires to run (e.g. the global descriptor table). */
    callq amd64_main

.global halt_core
halt_core:
    cli
    hlt

.global preempt_disable
preempt_disable:
    cli
    ret

.global preempt_enable
preempt_enable:
    sti
    ret
