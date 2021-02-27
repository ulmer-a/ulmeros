#pragma once

#include <util/types.h>

typedef struct
{
  ssize_t (*readblk)(size_t minor, char* buffer, size_t count, size_t lba);
  ssize_t (*writeblk)(size_t minor, char* buffer, size_t count, size_t lba);
} bd_ops_t;

typedef struct
{
  const char* name;
  const char* file_prefix;
  bd_ops_t bd_ops;
} bd_driver_t;

typedef struct
{

} bd_t;

void blockdev_init();

size_t bd_register_driver(bd_driver_t* bd_driver);

void bd_register(bd_t* blkdev);
