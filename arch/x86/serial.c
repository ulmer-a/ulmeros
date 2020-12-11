#include <arch/serial.h>
#include "ports.h"

#define SERIAL0_BASE 0x3f8

void serial0_init(int divisor)
{
  outb(SERIAL0_BASE + 1, 0x00);       // disable interrupts
  outb(SERIAL0_BASE + 3, 0x80);       // enable baud divisor
  outb(SERIAL0_BASE + 0, divisor);       // divisor low
  outb(SERIAL0_BASE + 1, 0x00);       // divisor high
  outb(SERIAL0_BASE + 3, 0x03);       // 8 bits, no parity, 1 stop bit
  outb(SERIAL0_BASE + 2, 0xC7);       // ?
  outb(SERIAL0_BASE + 4, 0x0B);       // ?
  outb(SERIAL0_BASE + 1, 0x01);       // Raise interrupt when data available
}

void serial0_write(char *buffer, size_t length)
{
  while (length--)
  {
    while ((inb(SERIAL0_BASE + 5) & 0x20) == 0);
    outb(SERIAL0_BASE, *buffer++);
  }
}
