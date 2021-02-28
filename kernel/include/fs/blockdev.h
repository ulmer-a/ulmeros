#pragma once

#include <util/types.h>

typedef struct
{
  ssize_t (*readblk)(void* drv_struct, size_t minor,
                     char* buffer, size_t count, uint64_t lba);
  ssize_t (*writeblk)(void* drv_struct, size_t minor,
                      char* buffer, size_t count, uint64_t lba);
} bd_ops_t;

typedef struct
{
  const char* name;
  const char* file_prefix;
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

size_t bd_register_driver(bd_driver_t* bd_driver);

void bd_register(bd_t* blkdev);
