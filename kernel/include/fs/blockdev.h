#pragma once

#include <types.h>

typedef struct
{
  ssize_t (*read)(char *buf, size_t count, size_t lba);
  ssize_t (*write)(char *buf, size_t count, size_t lba);
} bd_t;
