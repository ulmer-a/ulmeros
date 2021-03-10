#include <util/types.h>
#include <fs/blockdev.h>
#include <errno.h>
#include <debug.h>
#include <mm/memory.h>


extern list_t bd_list;
extern mutex_t bd_list_lock;


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
      mutex_unlock(&bd_list_lock);
      return SUCCESS;
    }
  }
  mutex_unlock(&bd_list_lock);
  return -ENOENT;
}
