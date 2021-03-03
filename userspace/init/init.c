

int _start()
{
  __asm__ volatile(
    "mov $0x01, %rax;"
    "int $0x80;"
  );
}
