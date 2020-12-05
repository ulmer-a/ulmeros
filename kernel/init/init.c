#include <fs/vfs.h>
#include <arch/platform.h>
#include <bus/pci.h>

void sysinit_task()
{
  debug(KMAIN, "Hello from the kernel initialization task!\n");
  /* initialize blockdevice manager */
  bd_init();

  /* perform a PCI scan, if present */
  pci_init();

  /* load the platform drivers */
  platform_init_drivers();

  /* initialize the virtual file system. this will load
   * all built in file system drivers. */
  vfs_init();

  /* mount the root filesystem */
  size_t root_major = 1, root_minor = 0;

  char buf[1024];
  bd_read(root_major, root_minor, buf, 2, 0);


  debug(KMAIN, "done\n");
}
