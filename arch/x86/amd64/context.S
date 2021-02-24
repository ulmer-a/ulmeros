.section .text

save_context:
    push %fs
    push %gs
    push %rbp
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    // set the pointer to the context in
    // the thread structure
    mov %rsp, %rdi
    call set_context

    mov %rsp, %rdi
    call x86_irq_handler

.global switch_task
switch_task:
    // get the next thread's context
    // and pop it into the registers
    call get_context
    mov %rax, %rsp

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    pop %rbp
    pop %gs
    pop %fs
    add $0x10, %rsp
    iretq

.macro m_irq_handler
.global irq\@
irq\@:
    .if (\@!=8) && (\@!=17) && (\@!=30) && (\@<10 || \@>14)
    pushq $0
    .else
    nop
    nop
    .endif
    pushq $\@
    jmp save_context
.endm

.rept 48
m_irq_handler
.endr
