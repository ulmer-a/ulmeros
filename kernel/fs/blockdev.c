#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/list.h>
#include <util/string.h>
#include <arch/common.h>
#include <debug.h>

static void* ramfs;
static size_t major_counter = 1;

static list_t driver_list;
static mutex_t driver_list_lock;

static list_t bd_list;
static mutex_t bd_list_lock;

void blockdev_init()
{
  debug(INIT, "initializing blockdevice manager\n");

  list_init(&driver_list);
  list_init(&bd_list);
  mutex_init(&driver_list_lock);
  mutex_init(&bd_list_lock);

  /* create a RAMFS for the device filesystem */
  ramfs = ramfs_create();
}

size_t bd_register_driver(bd_driver_t *bd_driver)
{
  const size_t major = atomic_add(&major_counter, 1);
  bd_driver->major = major;
  debug(BLKDEV, "registered driver '%s' with major id %zu\n",
        bd_driver->name, major);

  /* add the driver to the list of known drivers */
  mutex_lock(&driver_list_lock);
  list_add(&driver_list, bd_driver);
  mutex_unlock(&driver_list_lock);

  return major;
}

static void bdname(char* buffer, const char* prefix, size_t minor)
{
  strcpy(buffer, prefix);
  char ibuffer[32];
  strcpy(buffer, ultoa(minor, ibuffer, 10));
}

void bd_register(bd_t *blkdev)
{
  assert(blkdev->capacity, "blkdev must have non-zero capacity");

  bdname(blkdev->name, blkdev->driver->file_prefix, blkdev->minor);
  mutex_lock(&bd_list_lock);
  list_add(&bd_list, blkdev);
  mutex_unlock(&bd_list_lock);

  debug(BLKDEV, "registered block device (%zd, %zd): %s\n",
        blkdev->driver->major, blkdev->minor, blkdev->name);
}
