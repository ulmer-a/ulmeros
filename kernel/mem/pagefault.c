#include <memory.h>
#include <arch/context.h>
#include <definitions.h>
#include <vspace.h>


void page_fault()
{
  size_t fault_addr   = ctx_pf_addr();
  size_t fault_error  = ctx_pf_error();
  size_t fault_addr_virt = fault_addr >> 12;
  debug(PAGEFAULT, "%p: %s, %s, %s\n",
        (void*)fault_addr,
        fault_error & BIT(0) ? "present" : "non-present",
        fault_error & BIT(2) ? "user" : "kernel",
        fault_error & BIT(1) ? "write" : "read");

  if (fault_addr >= (size_t)kheap_start_
      && fault_addr < (size_t)kheap_break_)
  {
    if (vspace_map(VSPACE_KERNEL, fault_addr_virt,
               page_alloc(0), PG_KERNEL))
      return;
  }


  // CANNOT HANDLE
  assert(false, "unhandled page fault");
}
