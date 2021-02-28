#include <util/types.h>
#include <debug.h>

static const char* bool_str(int bool)
{
  return bool ? "yes" : "no";
}

void page_fault(size_t address, int present, int write, int user, int exec)
{
  debug(PAGEFAULT, "addr=%p, present=%s, write=%s, user=%s, exec=%s\n",
        address, bool_str(present), bool_str(write),
        bool_str(user), bool_str(exec));
  assert(false, "unhandled page fault");
}
