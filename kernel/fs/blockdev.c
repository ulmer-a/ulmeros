#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <fs/blockdev.h>
#include <util/list.h>
#include <util/string.h>
#include <arch/common.h>
#include <errno.h>
#include <debug.h>
#include <mm/memory.h>

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

  partscan_init();
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
  strcat(buffer, ultoa(minor, ibuffer, 10));
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

static ssize_t bd_read(void* drv, void* buffer, size_t size, uint64_t off)
{
  bd_t* bd = drv;
  if (!bd->driver->bd_ops.readblk)
    return -ENOTSUP;
  size_t lba = off / BLOCK_SIZE;
  size_t count = size / BLOCK_SIZE;
  ssize_t blks = bd->driver->bd_ops.readblk(
        bd->data, bd->minor, buffer, count, lba);
  if (blks < 0)
    return blks;
  return blks * BLOCK_SIZE;
}

static ssize_t bd_write(void* drv, void* buffer, size_t size, uint64_t off)
{
  bd_t* bd = drv;
  if (!bd->driver->bd_ops.writeblk)
    return -ENOTSUP;
  size_t lba = off / BLOCK_SIZE;
  size_t count = size / BLOCK_SIZE;
  ssize_t blks = bd->driver->bd_ops.writeblk(
        bd->data, bd->minor, buffer, count, lba);
  if (blks < 0)
    return blks;
  return blks * BLOCK_SIZE;
}

int bd_open(fd_t **fd_, size_t major, size_t minor)
{
  mutex_lock(&bd_list_lock);
  for (list_item_t* it = list_it_front(&bd_list);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    bd_t* bd = list_it_get(it);
    if (bd->minor == minor && bd->driver->major == major)
    {
      fd_t* fd = kmalloc(sizeof(fd_t));
      fd->fpos = 0;
      fd->fs_data = bd;
      fd->f_ops.read = bd_read;
      fd->f_ops.write = bd_write;
      *fd_ = fd;
      return SUCCESS;
    }
  }
  mutex_unlock(&bd_list_lock);
  return -ENOENT;
}
