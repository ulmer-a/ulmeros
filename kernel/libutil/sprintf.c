#include <util/string.h>

static char hex_char(size_t nibble)
{
  if (nibble > 0xF)
    return '*';
  if (nibble >= 0xA)
    return 'a' + nibble - 0xA;
  return '0' + nibble;
}

void _sprintf(char* dest, const char* fmt, va_list args)
{
  char* dest_ptr = dest;

  while (*fmt)
  {
    char c = *fmt++;
    if (c == '%')
    {
      switch (*fmt++)
      {
        case 's': {
          const char* str = va_arg(args, const char*);
          const size_t len = strlen(str);
          strcpy(dest_ptr, str);
          dest_ptr += len;
          break;
        }

        case 'z':
        case 'l':
        case '0':
        case 'u':
        case 'd':
        case 'x': {
          fmt -= 1;
          char buffer[32];
          int longValue = false;

          while ((c = *fmt++))
          {
            if (c == 'l')
            {
              longValue = true;
              continue;
            }

            if (c == 'z')
            {
              if (sizeof(size_t) > 4)
                longValue = true;
              continue;
            }

            if (c == 'd' && !longValue)
            {
              int value = va_arg(args, int);
              itoa(value, buffer, 10);
            }
            else if (c == 'u' && !longValue)
            {
              unsigned int value = va_arg(args, unsigned int);
              utoa(value, buffer, 10);
            }
            else if (c == 'x' && !longValue)
            {
              unsigned int value = va_arg(args, unsigned int);
              utoa(value, buffer, 16);
            }
            else if (c == 'd' && longValue)
            {
              int64_t value = va_arg(args, int64_t);
              itoa(value, buffer, 10);
            }
            else if (c == 'u' && longValue)
            {
              uint64_t value = va_arg(args, uint64_t);
              utoa(value, buffer, 10);
            }
            else if (c == 'x' && longValue)
            {
              uint64_t value = va_arg(args, uint64_t);
              utoa(value, buffer, 16);
            }

            const size_t len = strlen(buffer);
            strcpy(dest_ptr, buffer);
            dest_ptr += len;
            break;
          }
          break;
        }

        case 'p': {
          *dest_ptr++ = '0';
          *dest_ptr++ = 'x';
          ssize_t ptr_nibbles = sizeof(void*) * 2;
          const size_t ptr = va_arg(args, size_t);
          while (--ptr_nibbles >= 0)
          {
            size_t nibble = (ptr >> (ptr_nibbles * 4)) & 0xf;
            *dest_ptr++ = hex_char(nibble);
          }
          break;
        }

        case 'q': {
          *dest_ptr++ = '0';
          *dest_ptr++ = 'x';
          ssize_t ptr_nibbles = sizeof(void*) * 4;
          const uint64_t ptr = va_arg(args, uint64_t);
          while (--ptr_nibbles >= 0)
          {
            size_t nibble = (ptr >> (ptr_nibbles * 4)) & 0xf;
            *dest_ptr++ = hex_char(nibble);
          }
          break;
        }

        case 'c':
          *dest_ptr++ = va_arg(args, int);
          break;

        case '%':
          *dest_ptr++ = '%';
          break;

        default:
          break;
      }
    }
    else
    {
      *dest_ptr++ = c;
    }
  }

  *dest_ptr = 0;
}

void sprintf(char* dest, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  _sprintf(dest, fmt, args);
  va_end(args);
}
