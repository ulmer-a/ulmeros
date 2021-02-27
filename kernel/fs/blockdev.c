#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <arch/common.h>
#include <debug.h>

static void* ramfs;
static size_t major_counter = 1;

void blockdev_init()
{
  debug(INIT, "initializing blockdevice manager\n");

  /* create a RAMFS for the device filesystem */
  ramfs = ramfs_create();
}

size_t bd_register_driver(bd_driver_t* bd_driver)
{
  (void)bd_driver;

  const size_t major = atomic_add(&major_counter, 1);
  debug(BLKDEV, "registered driver '%s' with major id %zu\n",
        bd_driver->name, major);
  return major;
}
