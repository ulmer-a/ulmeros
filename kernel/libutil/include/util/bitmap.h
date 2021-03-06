#pragma once

#include <util/types.h>

#define BITMAP_BYTE_SIZE(b) ((b % 8 == 0) ? (b >> 3) : (b >> 3) + 1)

typedef struct
{
  size_t size;
  uint8_t* bitmap;
} bitmap_t;

#define BITMAP_INIT(b, array) { b, array }


void bitmap_init(bitmap_t* bmp, size_t size, int value);

int bitmap_get(bitmap_t *bmp, size_t index);
void bitmap_set(bitmap_t* bmp, size_t index);
void bitmap_clr(bitmap_t* bmp, size_t index);
size_t bitmap_find_free(bitmap_t* bmp);
size_t bitmap_find_n_free(bitmap_t* bmp, size_t n);
