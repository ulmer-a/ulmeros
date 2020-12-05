#include <fs/blockdev.h>
#include <list.h>
#include <mutex.h>
#include <types.h>
#include <fs/vfs.h>

static size_t  bd_major_counter = 1;
static list_t* bd_driver_list;
static list_t* bd_device_list;
static mutex_t bd_driver_list_lock = MUTEX_INITIALIZER;
static mutex_t bd_device_list_lock = MUTEX_INITIALIZER;

void bd_init()
{
  debug(BLKDEV, "setting up block device manager\n");
  bd_driver_list = list_init();
  bd_device_list = list_init();

  iosched_init();
}

size_t bd_register_driver(bd_driver_t *driver_info)
{
  mutex_lock(&bd_driver_list_lock);
  driver_info->major = bd_major_counter++;
  list_add(bd_driver_list, driver_info);
  mutex_unlock(&bd_driver_list_lock);

  debug(BLKDEV, "register blockdev driver %zd -> %s\n",
        driver_info->major, driver_info->name);
  return driver_info->major;
}

static bd_driver_t* bd_get_driver(size_t major)
{
  mutex_lock(&bd_driver_list_lock);
  bd_driver_t* ret = NULL;
  for (int i = 0; i < list_size(bd_driver_list); i++)
  {
    bd_driver_t* driver = list_get(bd_driver_list, 0);
    if (driver->major == major)
    {
      ret = driver;
      break;
    }
  }
  mutex_unlock(&bd_driver_list_lock);
  return ret;
}

bd_t* bd_get(size_t major, size_t minor)
{
  mutex_lock(&bd_device_list_lock);
  bd_t* found = NULL;
  for (size_t i = 0; i < list_size(bd_device_list); i++)
  {
    bd_t* bd = list_get(bd_device_list, i);
    if (bd->driver->major == major &&
        bd->minor == minor)
    {
      found = bd;
      break;
    }
  }
  mutex_unlock(&bd_device_list_lock);
  return found;
}

void bd_register(bd_t *blockdev)
{
  debug(BLKDEV, "register block device %s%zd (%zd, %zd)\n",
        blockdev->driver->file_prefix, blockdev->minor,
        blockdev->driver->major, blockdev->minor);

  mutex_lock(&bd_device_list_lock);
  list_add(bd_device_list, blockdev);
  mutex_unlock(&bd_device_list_lock);
}

ssize_t bd_read(size_t major, size_t minor,
             char* buf, size_t count, size_t lba)
{
  bd_driver_t* driver = bd_get_driver(major);
  return driver->fops.read(minor, buf, count, lba);
}

ssize_t bd_write(size_t major, size_t minor,
             char* buf, size_t count, size_t lba)
{
  bd_driver_t* driver = bd_get_driver(major);
  return driver->fops.write(minor, buf, count, lba);
}
