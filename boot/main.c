#include "multiboot.h"
#include "boot32.h"

extern void go_longmode();
extern void set_error_msg(const char *msg);
extern void print_error();


static uint64_t memCheck(struct mb_struct *mb)
{
    uint64_t physical_size = 0;

    struct mmape_struct *end =
            (struct mmape_struct*)((char*)mb->mmap + mb->mmap_length);

    for (struct mmape_struct *entry = mb->mmap;
         entry < end;
         entry = (struct mmape_struct*)((char*)entry + entry->entry_size + 4))
    {
        physical_size += entry->size;
    }

    return physical_size;
}

extern char _binary_kernel64_bin_start;
extern char _binary_kernel64_bin_end;

void main32(struct mb_struct *mb)
{
    clearBss();

    printMessage("Ulmer OS boot32");

    ramSize = memCheck(mb);

    // copy kernel binary to entry address
    uint32_t binary_size = (uint32_t)&_binary_kernel64_bin_end - (uint32_t)&_binary_kernel64_bin_start;
    memcpy(ENTRY64_ADDR, &_binary_kernel64_bin_start, binary_size);

    // actually entery 64 bit mode an jump to kernel
    go_longmode();
}
