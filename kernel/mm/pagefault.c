#include <util/types.h>
#include <debug.h>

void page_fault(size_t address, int present, int write, int user, int exec)
{
  assert(false, "unhandled page fault");
}
