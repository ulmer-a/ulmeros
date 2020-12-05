#pragma once

#include <types.h>

#define IO_SIZE 512

typedef struct
{
  ssize_t (*read)(size_t minor, char *buf,
                  size_t count, uint64_t lba);
  ssize_t (*write)(size_t minor, char *buf,
                   size_t count, uint64_t lba);
} bd_fops_t;

typedef struct
{
  const char* name;
  const char* file_prefix;
  size_t major;
  bd_fops_t fops;
} bd_driver_t;

typedef struct
{
  const bd_driver_t* driver;
  size_t minor;
  size_t capacity;
} bd_t;

void bd_init();
void parts_init();
void iosched_init();

void partscan(bd_t* disk);

size_t bd_register_driver(bd_driver_t *driver_info);

void bd_register(bd_t* blockdev);

bd_t* bd_get(size_t major, size_t minor);

ssize_t bd_read(size_t major, size_t minor,
             char* buf, size_t count, size_t lba);
ssize_t bd_write(size_t major, size_t minor,
             char* buf, size_t count, size_t lba);

