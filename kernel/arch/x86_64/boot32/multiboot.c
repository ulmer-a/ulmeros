
#include "boot32.h"

void get_multiboot_info(multiboot_t* mb)
{
  /* get the kernel command line */
  boot_info.cmdline_ptr = 0;
  if (mb->flags & BIT(2))
  {
    size_t length = strlen((const char*)mb->cmdline);
    char* cmdline = kmalloc(length + 1);
    strcpy(cmdline, (const char*)mb->cmdline);
    boot_info.cmdline_ptr = (uint64_t)cmdline + IDENT_OFFSET;
    debug("kernel cmdline: \"%s\"\n", cmdline);
  }

  /* get installed modules, the first one must always be
   * the initial ramdisk. all other modules are ignored. */
  boot_info.ramdisk_ptr = 0;
  boot_info.ramdisk_size = 0;
  if (mb->flags & BIT(3) && mb->mods_count > 0)
  {
    debug("multiboot: found %zu modules (initrd)\n", mb->mods_count);
    multiboot_mods_t* initrd = (multiboot_mods_t*)mb->mods;
    size_t initrd_size = initrd->mod_end - initrd->mod_start;
    void* initrd_ptr = kmalloc(initrd_size);
    memcpy(initrd_ptr, (void*)initrd->mod_start, initrd_size);

    boot_info.ramdisk_ptr = (uint64_t)initrd_ptr + IDENT_OFFSET;
    boot_info.ramdisk_size = initrd_size;
  }

  /* get graphics mode information */
  // not implemented yet.
}
