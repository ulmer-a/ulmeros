#include <memory.h>
#include <mutex.h>
#include <kstring.h>
#include <vspace.h>

size_t          total_pages_;
uint8_t*        refcounter_ = NULL;
static mutex_t  refcounter_lock_ = MUTEX_INITIALIZER;

static void page_clear(size_t phys)
{
  void* addr = vspace_get_page_ptr(phys);
  memset(addr, 0, PAGE_SIZE);
}

size_t page_alloc(int flags)
{
  (void)flags;
  assert(refcounter_, "page_alloc(): uninitialized");

  mutex_lock(&refcounter_lock_);
  for (size_t i = 0; i < total_pages_; i++)
  {
    if (refcounter_[i] == 0)
    {
      refcounter_[i]++;
      mutex_unlock(&refcounter_lock_);

      page_clear(i);
      return i;
    }
  }
  mutex_unlock(&refcounter_lock_);

  // out of memory!
  assert(false, "page_alloc(): out of memory");
  return 0;
}

void page_release(size_t page)
{
  assert(refcounter_, "page_release(): uninitialized");
  assert(page < total_pages_, "page_release(): page >= total_pages");

  mutex_lock(&refcounter_lock_);
  assert(refcounter_[page] > 0, "page_release(): refcount is zero");
  refcounter_[page] -= 1;
  mutex_unlock(&refcounter_lock_);
}
