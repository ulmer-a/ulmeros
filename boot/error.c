#define VIDEO_START ((void*)0xb8000)
#define VIDEO_SIZE  80*25*2

static void clear_screen()
{
  char *vmem_ptr = VIDEO_START;
  unsigned vmem_size = VIDEO_SIZE;

  while (vmem_size--)
    *vmem_ptr++ = 0;
}
void printMessage(const char *msg)
{
  clear_screen();

  const char *msg_ptr = msg;
  char *vmem_ptr = VIDEO_START;

  while (*msg_ptr)
  {
    *vmem_ptr++ = *msg_ptr++;
    *vmem_ptr++ = 0x0f;
  }
}

void print_error()
{
  printMessage("CPU does not support x86 long mode\n");
}
