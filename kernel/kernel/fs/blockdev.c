#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/list.h>
#include <util/string.h>
#include <arch/common.h>
#include <errno.h>
#include <debug.h>
#include <mm/memory.h>

#define BLKDEV_MODE   {               \
  .u_r = 1, .u_w = 1, .u_x = 0,       \
  .g_r = 1, .g_w = 1, .g_x = 0,       \
  .o_r = 0, .o_w = 0, .o_x = 0,       \
  .sticky = 0,                        \
  .setgid = 0,                        \
  .setuid = 0                         \
}

static size_t major_counter = 1;

static list_t driver_list;
static mutex_t driver_list_lock;

list_t bd_list;
mutex_t bd_list_lock;

void blockdev_init()
{
  debug(INIT, "initializing blockdevice manager\n");

  list_init(&driver_list);
  list_init(&bd_list);
  mutex_init(&driver_list_lock);
  mutex_init(&bd_list_lock);

  /* initialize the partscan driver, which scans each
   * newly registered blockdevice for a partition table */
  partscan_init();
}

size_t bd_register_driver(bd_driver_t *bd_driver)
{
  /* on registration of a driver, a major number is assigned. */
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

static void try_mknod_blkdevice(bd_t* blkdev)
{
  char path_buffer[256];
  static const fmode_t blkdev_mode = BLKDEV_MODE;
  sprintf(path_buffer, "/dev/%s", blkdev->name);
  int status = vfs_mknod(NULL, path_buffer, F_BLOCK, blkdev_mode,
            blkdev->driver->major, blkdev->minor);
  if (status < 0)
    debug(BLKDEV, "error: cannot create %s: %s\n", strerror(-status));
  else
    debug(BLKDEV, "add new block device %s\n", path_buffer);
}

void bd_register(bd_t *blkdev)
{
  assert(blkdev->capacity, "blkdev must have non-zero capacity");
  assert(blkdev->driver, "driver field must be initialized");

  mutex_lock(&bd_list_lock);
  list_add(&bd_list, blkdev);
  mutex_unlock(&bd_list_lock);

  debug(BLKDEV, "registered block device (%zd, %zd): %s\n",
        blkdev->driver->major, blkdev->minor, blkdev->name);

  if (vfs_initialized)
    try_mknod_blkdevice(blkdev);

  /* trigger a partition scan */
  partscan(blkdev);
}

int bd_get_by_name(const char *name, size_t *major, size_t *minor)
{
  mutex_lock(&bd_list_lock);
  for (list_item_t* it = list_it_front(&bd_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    bd_t* bd = list_it_get(it);
    if (strcmp(name, bd->name) == 0)
    {
      *major = bd->driver->major;
      *minor = bd->minor;
      mutex_unlock(&bd_list_lock);
      return true;
    }
  }
  mutex_unlock(&bd_list_lock);
  return false;
}


void blockdev_mknodes()
{
  mutex_lock(&bd_list_lock);
  for (list_item_t* it = list_it_front(&bd_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    bd_t* bd = list_it_get(it);
    try_mknod_blkdevice(bd);
  }
  mutex_unlock(&bd_list_lock);
}
