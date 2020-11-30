#pragma once

#include <version.h>

#define NULL      ((void*)0)
#define BIT(x)    (1 << (x))
#define _init

typedef unsigned long   size_t;
typedef long            ssize_t;

typedef unsigned long long  uint64_t;
typedef long long           int64_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;

struct boot_info_;
typedef struct boot_info_ boot_info_t;

#define true 1
#define false 0

#include <debug.h>

void debug(int level, const char *fmt, ...);
void panic();

#define assert(x, y) if (!(x)) { \
  debug(ASSERT, "assertion failed in %s on line %s: %s\n", \
    __FILE__, __LINE__, (y)); \
  panic(); \
}
