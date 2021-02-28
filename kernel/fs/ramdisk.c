/*
 * UlmerOS ramdisk driver
 * Copyright (C) 2021 Alexander Ulmer
 */

#include <fs/vfs.h>
#include <fs/blockdev.h>
#include <arch/common.h>
#include <util/string.h>
#include <mm/memory.h>
#include <debug.h>
#include <errno.h>

typedef struct
{
  void* ramdisk;
  size_t ramdisk_size;
  size_t minor;
  mutex_t rd_lock;
} rd_t;

static size_t initrd_major;
static size_t minor_counter = 0;

static ssize_t initrd_read(void* drv, size_t minor,
                           char* buffer, size_t count, uint64_t lba)
{
  rd_t* rd = drv;

  if (lba + count > rd->ramdisk_size)
    count = rd->ramdisk_size - lba;

  memcpy(buffer, rd->ramdisk + lba * BLOCK_SIZE, count * BLOCK_SIZE);
  return count * BLOCK_SIZE;
}

static ssize_t initrd_write(void* drv, size_t minor,
                            char* buffer, size_t count, uint64_t lba)
{
  rd_t* rd = drv;
  mutex_lock(&rd->rd_lock);

  if (lba + count > rd->ramdisk_size)
    count = rd->ramdisk_size - lba;

  memcpy(rd->ramdisk + lba * BLOCK_SIZE, buffer, count * BLOCK_SIZE);
  mutex_unlock(&rd->rd_lock);
  return count * BLOCK_SIZE;
}

static bd_driver_t initrd_driver = {
  .name = "ramdisk",
  .file_prefix = "rd",
  .bd_ops = {
    .readblk = initrd_read,
    .writeblk = initrd_write
  }
};

void ramdisk_init()
{
  initrd_major = bd_register_driver(&initrd_driver);
}

bd_t* ramdisk_install(void *ramdisk, size_t ramdisk_size)
{
  assert(ramdisk, "invalid ramdisk ptr");

  rd_t* rd = kmalloc(sizeof(rd_t));
  rd->ramdisk = ramdisk;
  rd->ramdisk_size = ramdisk_size >> 9;
  rd->minor = atomic_add(&minor_counter, 1);

  bd_t* bd = kmalloc(sizeof(bd_t));
  bd->capacity = rd->ramdisk_size / BLOCK_SIZE;
  bd->driver = &initrd_driver;
  bd->data = rd;
  bd->minor = rd->minor;

  debug(BLKDEV, "(%zu, %zu): installing ramdisk, size=%zu blocks\n",
        initrd_major, rd->minor, rd->ramdisk_size / BLOCK_SIZE);
  bd_register(bd);
  return bd;
}
