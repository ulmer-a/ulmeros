.section .text

.global gdt_write
gdt_write:
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    jmp $0x08, $.write_gdt
.write_gdt:
    ret

.global tss_write
tss_write:
    mov $0x2b, %ax
    ltr %ax
    ret

.global enable_paging
enable_paging:
    // load address of PML4 into %cr3
    mov 4(%esp), %eax
    mov %eax, %cr3

    // enable physical address extension (PAE)
    mov %cr4, %eax
    btsl $5, %eax
    mov %eax, %cr4

    // enable long mode
    mov $0xc0000080, %ecx
    rdmsr
    btsl $8, %eax
    wrmsr

    // finally enable paging itself
    mov %cr0, %eax
    btsl $31, %eax
    mov %eax, %cr0

    ret
