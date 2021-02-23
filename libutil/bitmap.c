#include <util/bitmap.h>
#include <util/string.h>
#include <util/system.h>

void bitmap_init(bitmap_t* bmp, size_t size, int value)
{
  assert(bmp, "bmp has to be a valid ptr");

  const size_t bytes = BITMAP_BYTE_SIZE(size);
  bmp->bitmap = kmalloc(bytes);
  bmp->size = size;
  memset(bmp->bitmap, value, bytes);
}

int bitmap_get(bitmap_t* bmp, size_t index)
{
  assert(index < bmp->size, "bitmap_get(): out of bounds");

  size_t byte = index / 8;
  size_t bit = index % 8;

  return (bmp->bitmap[byte] & BIT(bit)) ? 1 : 0;
}

void bitmap_set(bitmap_t* bmp, size_t index)
{
  assert(index < bmp->size, "bitmap_set(): out of bounds");

  size_t byte = index / 8;
  size_t bit = index % 8;

  bmp->bitmap[byte] |= BIT(bit);
}

void bitmap_clr(bitmap_t *bmp, size_t index)
{
  assert(index < bmp->size, "bitmap_clr(): out of bounds");

  size_t byte = index / 8;
  size_t bit = index % 8;

  bmp->bitmap[byte] &= ~BIT(bit);
}

size_t bitmap_find_free(bitmap_t *bmp)
{
  size_t bit = 0;
  for (size_t i = 0; i < BITMAP_BYTE_SIZE(bmp->size); i++)
  {
    if (bmp->bitmap[i] != 0xff)
    {
      bit = i * 8;

      for (int j = 0; j < 8; j++)
      {
        if ((bmp->bitmap[i] & BIT(j)) == 0)
        {
          bit += j;
          break;
        }
      }

      return bit;
    }
  }
  return (size_t)-1;
}
