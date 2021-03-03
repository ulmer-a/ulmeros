#pragma once

#define NULL ((void*)0)
#define BIT(x) (1ul << (x))
#define MMIO32(addr) ((volatile unsigned int*)(MMIO_BASE+(addr)))

#define true 1
#define false 0

typedef unsigned long size_t;
typedef long ssize_t;

typedef unsigned long long uint64_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define max(a, b) (((a) > (b)) ? (a) : (b))
