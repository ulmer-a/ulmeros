#include <amd64/boot.h>
#include "boot32.h"

extern void jmp64(boot_info_t* bootInfo);
extern void* setupPaging();
extern void* setupGdt();

uint64_t ramSize;

void go_longmode()
{
    void* pml4Page = setupPaging();

    // enable paging
    __asm__ volatile (
        "mov $0xc0000080, %ecx;"
        "rdmsr;"
        "or $(1 << 8), %eax;"
        "wrmsr;"
        "mov %cr0, %eax;"
        "or $(1 << 31), %eax;"
        "mov %eax, %cr0"
    );

    // setup global descriptor table
    void* gdtPage = setupGdt();

    // gdtPage containst the global descriptor table but
    // has some left over space. we use it to store the
    // Boot64Struct boot info structure which stores system
    // information for the 64bit kernel
    //unsigned long long userBreakPageOffset = 0xffff800000000000ull >> 12;
    boot_info_t* bootInfo = gdtPage + 0x800;
    bootInfo->gdtPage = (uint32_t)gdtPage / PAGE_SIZE;
    bootInfo->ramPages = ramSize / PAGE_SIZE;
    bootInfo->heapPageCount = getHeapSize();
    bootInfo->heapStartPage = getHeapStart();
    bootInfo->kernelStartPage = ((unsigned long)ENTRY64_ADDR >> 12);
    bootInfo->kernelPageCount = binary_size;
    bootInfo->pml4Page = (unsigned long)pml4Page >> 12;
    bootInfo->mmap = (unsigned long)mmap_ptr;
    bootInfo->mmap_length = mmap_len;

    // perform the jump to long mode
    jmp64(bootInfo);
}
