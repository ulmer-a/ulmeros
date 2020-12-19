.section .text

.global _syscall
_syscall:
    int $0x80
    ret
