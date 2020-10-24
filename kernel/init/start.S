.section .text.init

    movabs $_start64, %rax
    jmp *%rax

.global _start64
_start64:
    cli

    // setup initializiation stack
    movabs $0xffff800000080000, %rsp

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    callq main64

    cli
    hlt
