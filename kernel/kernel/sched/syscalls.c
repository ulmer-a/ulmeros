#include <syscalls.h>

#define UNIM (void*)0xdeadbeef

void* syscall_table[] = {
  UNIM,            // 0x00
  sys_exit,       // 0x01
  sys_read,       // 0x02
  sys_write,      // 0x03
  sys_close,      // 0x04
  UNIM,           // 0x05
  UNIM,           // 0x06
  UNIM,           // 0x07
  UNIM,           // 0x08
  UNIM,           // 0x09
  UNIM,           // 0x0a
  UNIM,           // 0x0b
  UNIM,           // 0x0c
  UNIM,           // 0x0d
  sys_sbrk,       // 0x0e
  UNIM,           // 0x0f
  UNIM,           // 0x10
  UNIM,           // 0x11
  UNIM,           // 0x12
  UNIM,           // 0x13
};

size_t syscall_count()
{
  return sizeof(syscall_table) / sizeof(void*);
}
