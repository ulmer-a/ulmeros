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
    movabs $0xffff800000080000, %rsp

    /* a pointer to the boot info structure is
     * located in the RDI register. save it. */
    push %rdi

    /* call the architecture initialization function. this
     * will setup anything architecture-specific the kernel
     * reqires to run (e.g. the global descriptor table). */
    callq arch_init


    /* fill the segment registers with the right
     * values regarding the global descriptor table */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    /* jump to the kernel main initialization routine. kmain()
     * will start the scheduler and perform the first context
     * switch, so it never returns. */
    pop %rdi
    jmp kmain
