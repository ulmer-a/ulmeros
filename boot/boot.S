.section .text.mboot

jmp _start32

.balign 4
multiboot_header:
    .long 0x1badb002   // magic
    .long 0x2          // flags: flat binary
    .long -0x1badb004   // depends on flags!!!
    .long (multiboot_header)
    .long 0x100000     // load addr
    .long 0            // whole image
    .long 0            // don't clear BSS
    .long 0x100000     // entry addr

.global _start32
_start32:
    mov $0x1000000, %esp

    // save multiboot struct address
    push %ebx

    // enable CPU cache
    mov %cr0, %eax
    and $0x9FFFFFFF, %eax
    mov %eax, %cr0

    // check if CPUID is supported
    pushf
    pop %eax
    mov %eax, %ecx
    xor $(1 << 21), %eax
    push %eax
    popf
    push %ecx
    popf
    xor %ecx, %eax
    //jz .error

    // check if extended function is present
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    //jb .error

    // check if long mode is available
    mov $0x80000001, %eax
    cpuid
    test $(1 << 29), %edx
    //jz .error // comment out to debug

    // setup_longmode(struct mb_struct)
    // ebx points to struct multiboot
    call main32

    cli
    hlt

.error:
    call print_error

    cli
    hlt

.global jmp64
jmp64:
    mov 0x4(%esp), %edi
    ljmp $0x8, $0x1000000
