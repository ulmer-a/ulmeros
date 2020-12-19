

void* memmove(void* dest, void* src, unsigned long n)
{
  unsigned char* s1 = dest;
  unsigned char* s2 = src;
  while (n--)
    *s1++ = *s2++;
  return dest;
}
