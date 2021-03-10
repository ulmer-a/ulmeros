#pragma once

#include <util/types.h>
#include <fs/vfs.h>

typedef struct
{
  ssize_t (*readblk)(void* drv_struct, size_t minor,
                     char* buffer, size_t count, uint64_t lba);
  ssize_t (*writeblk)(void* drv_struct, size_t minor,
                      char* buffer, size_t count, uint64_t lba);
  const char* (*get_prefix)(void* drv_struct);
} bd_ops_t;

typedef struct
{
  const char* name;
  const char* prefix;
  bd_ops_t bd_ops;
  size_t major;
} bd_driver_t;

typedef struct
{
  size_t minor;       // minor number
  size_t capacity;    // capacity in blocks
  bd_driver_t* driver;       // driver structure
  void* data;
  char name[32];
} bd_t;

void blockdev_init();
void blockdev_mknodes();

int bd_get_by_name(const char* name, size_t* major, size_t* minor);

int bd_open(fd_t** fd_, size_t major, size_t minor);

size_t bd_register_driver(bd_driver_t* bd_driver);

void bd_register(bd_t* blkdev);

void partscan_init();

void partscan(bd_t* blkdev);
