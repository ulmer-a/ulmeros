#include <arch/context.h>
#include <debug.h>

context_t* current_context = NULL;

/* whenever a context switch occurs, the old context is pushed
 * onto the thread's kernel stack by architecture specific code.
 * then, save_context() is called, to save the pointer to the
 * thread's data  */
void set_context(context_t* ctx)
{
  current_context = ctx;
}

/* whenever control flow returns from an interrupt, exception or
 * system call handler, the get_context() function is called. it
 * should return a pointer to the context of the thread, to which
 * control flow is returning.  */
context_t* get_context()
{
  assert(current_context, "irq return but no context to switch to!");
  return current_context;
}
