#include <fs/vfs.h>

void sysinit_task()
{
  debug(KMAIN, "Welcome from the first kernel task!\n");

  /* initialize the virtual file system. this will load
   * common file system drivers and mount the root fs. */
  vfs_init();

  debug(KMAIN, "done");
  for (;;)
    __asm__ volatile ("hlt");
}
