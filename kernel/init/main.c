#include <debug.h>
#include <cmdline.h>

void kmain(const char *cmdline)
{
  debug(INIT, "reached kmain()\n");
  cmdline_parse(cmdline);

  // kernel init
}
